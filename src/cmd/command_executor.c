#include "interpreter/basic_interpreter.h"

int parse_line(Interpreter *interp, const char *line_text) {
    if (interp->line_count >= MAX_LINES) {
        print_error(interp, "Too many lines");
        return 0;
    }

    const char *ptr = line_text;
    while (isspace(*ptr)) ptr++;
    if (!*ptr || *ptr == '\n') return 1;

    Line *line = &interp->lines[interp->line_count];

    // get the line number
    if (isdigit(*ptr)) {
        line->line_number = atoi(ptr);
        while (isdigit(*ptr)) ptr++;
        while (isspace(*ptr)) ptr++;
    } else {
        line->line_number = interp->line_count * 10 + 10;
    }

    line->text = malloc(strlen(ptr) + 1);
    strcpy(line->text, ptr);

    line->tokens = tokenize(ptr, &line->token_count);

    interp->line_count++;
    return 1;
}

int execute_print(Interpreter *interp, Token *tokens, int token_count, int start) {
    if (start >= token_count) {
        printf("\n");
        return 0;
    }

    int i = start;
    while (i < token_count) {
        // try to find the end of the expression
        int expr_end = i;
        int paren_level = 0;

        while (expr_end < token_count) {
            if (tokens[expr_end].type == TOKEN_DELIMITER && strcmp(tokens[expr_end].text, "(") == 0) {
                paren_level++;
            } else if (tokens[expr_end].type == TOKEN_DELIMITER && strcmp(tokens[expr_end].text, ")") == 0) {
                paren_level--;
            } else if (paren_level == 0 && tokens[expr_end].type == TOKEN_DELIMITER &&
                      (strcmp(tokens[expr_end].text, ",") == 0 || strcmp(tokens[expr_end].text, ";") == 0)) {
                break;
            }
            expr_end++;
        }

        if (expr_end > i) {
            Value result = evaluate_expression(interp, tokens, i, expr_end - 1);
            if (result.type == VALUE_NUMBER) {
                printf("%.6g", result.data.number);
            } else if (result.type == VALUE_STRING) {
                printf("%s", result.data.string);
            }
            cleanup_value(&result);
        }

        // separator block
        if (expr_end < token_count && tokens[expr_end].type == TOKEN_DELIMITER) {
            if (strcmp(tokens[expr_end].text, ",") == 0) {
                printf("\t");
            } else if (strcmp(tokens[expr_end].text, ";") == 0) {
                // nothing happens, no separator has been found
            }
            i = expr_end + 1;
        } else {
            break;
        }
    }

    printf("\n");
    return 1;
}

int execute_let(Interpreter *interp, Token *tokens, int token_count, int start) {
    if (start + 2 >= token_count || tokens[start].type != TOKEN_VARIABLE ||
        tokens[start + 1].type != TOKEN_OPERATOR || tokens[start + 1].operator != OP_EQUAL) {
        print_error(interp, "Invalid LET statement");
        return 0;
    }

    char *var_name = tokens[start].text;
    Value value = evaluate_expression(interp, tokens, start + 2, token_count - 1);
    set_variable(interp, var_name, value);

    return 1;
}

int execute_input(Interpreter *interp, Token *tokens, int token_count, int start) {
    char input_buffer[MAX_INPUT_LENGTH];

    // check prompt as char*
    int var_start = start;
    if (start < token_count && tokens[start].type == TOKEN_STRING) {
        printf("%s", tokens[start].value.data.string);
        var_start = start + 1;
        if (var_start < token_count && tokens[var_start].type == TOKEN_DELIMITER &&
            strcmp(tokens[var_start].text, ";") == 0) {
            var_start++;
        }
    }

    if (var_start >= token_count || tokens[var_start].type != TOKEN_VARIABLE) {
        print_error(interp, "INPUT requires a variable");
        return 0;
    }

    printf("? ");
    if (fgets(input_buffer, sizeof(input_buffer), stdin)) {
        char *newline = strchr(input_buffer, '\n');
        if (newline) *newline = '\0';

        // num?
        char *endptr;
        double num = strtod(input_buffer, &endptr);

        Value value;
        if (*endptr == '\0' && endptr != input_buffer) {
            // valid number found
            value = create_number_value(num);
        } else {
            // it's a string
            value = create_string_value(input_buffer);
        }

        set_variable(interp, tokens[var_start].text, value);
    }

    return 1;
}

int execute_if(Interpreter *interp, Token *tokens, int token_count, int start) {
    int then_pos = -1;
    for (int i = start; i < token_count; i++) {
        if (tokens[i].type == TOKEN_COMMAND && tokens[i].command == CMD_THEN) {
            then_pos = i;
            break;
        }
    }

    if (then_pos == -1) {
        print_error(interp, "IF without THEN");
        return 0;
    }

    Value condition = evaluate_expression(interp, tokens, start, then_pos - 1);
    const int is_true = condition.type == VALUE_NUMBER && condition.data.number != 0;
    cleanup_value(&condition);

    if (is_true) {
        // THEN exec
        if (then_pos + 1 < token_count) {
            if (tokens[then_pos + 1].type == TOKEN_NUMBER) {
                // GOTO line number
                int line_num = (int)tokens[then_pos + 1].value.data.number;
                int line_index = find_line_by_number(interp, line_num);
                if (line_index != -1) {
                    interp->current_line = line_index + 1;
                } else {
                    print_error(interp, "Line number not found");
                    return 0;
                }
            } else {
                // exec remaining tokens as a statement
                return execute_line_tokens(interp, tokens, token_count, then_pos + 1);
            }
        }
    }

    return 1;
}

int execute_for(Interpreter *interp, Token *tokens, int token_count, int start) {
    if (start + 4 >= token_count || tokens[start].type != TOKEN_VARIABLE ||
        tokens[start + 1].type != TOKEN_OPERATOR || tokens[start + 1].operator != OP_EQUAL) {
        print_error(interp, "Invalid FOR statement");
        return 0;
    }

    char *var_name = tokens[start].text;

    // look for TO keyword
    int to_pos = -1;
    for (int i = start + 2; i < token_count; i++) {
        if (tokens[i].type == TOKEN_COMMAND && tokens[i].command == CMD_TO) {
            to_pos = i;
            break;
        }
    }

    if (to_pos == -1) {
        print_error(interp, "FOR without TO");
        return 0;
    }

    // STEP? (optional)
    int step_pos = -1;
    for (int i = to_pos + 1; i < token_count; i++) {
        if (tokens[i].type == TOKEN_COMMAND && tokens[i].command == CMD_STEP) {
            step_pos = i;
            break;
        }
    }

    Value start_val = evaluate_expression(interp, tokens, start + 2, to_pos - 1);
    if (start_val.type != VALUE_NUMBER) {
        print_error(interp, "FOR start value must be numeric");
        cleanup_value(&start_val);
        return 0;
    }

    int end_expr_end = (step_pos != -1) ? step_pos - 1 : token_count - 1;
    Value end_val = evaluate_expression(interp, tokens, to_pos + 1, end_expr_end);
    if (end_val.type != VALUE_NUMBER) {
        print_error(interp, "FOR end value must be numeric");
        cleanup_value(&start_val);
        cleanup_value(&end_val);
        return 0;
    }

    double step = 1.0;
    if (step_pos != -1) {
        Value step_val = evaluate_expression(interp, tokens, step_pos + 1, token_count - 1);
        if (step_val.type != VALUE_NUMBER) {
            print_error(interp, "FOR step value must be numeric");
            cleanup_value(&start_val);
            cleanup_value(&end_val);
            cleanup_value(&step_val);
            return 0;
        }
        step = step_val.data.number;
        cleanup_value(&step_val);
    }

    set_variable(interp, var_name, start_val);

    // push onto FOR stack
    if (interp->for_stack_top >= MAX_FOR_STACK - 1) {
        print_error(interp, "FOR stack overflow");
        cleanup_value(&start_val);
        cleanup_value(&end_val);
        return 0;
    }

    ForLoop *loop = &interp->for_stack[++interp->for_stack_top];
    strcpy(loop->variable, var_name);
    loop->start = start_val.data.number;
    loop->end = end_val.data.number;
    loop->step = step;
    loop->line_index = interp->current_line - 1;

    cleanup_value(&start_val);
    cleanup_value(&end_val);
    return 1;
}

int execute_next(Interpreter *interp) {
    if (interp->for_stack_top < 0) {
        print_error(interp, "NEXT without FOR");
        return 0;
    }

    ForLoop *loop = &interp->for_stack[interp->for_stack_top];

    Variable *var = get_variable(interp, loop->variable);
    if (!var || var->value.type != VALUE_NUMBER) {
        print_error(interp, "FOR variable not found");
        return 0;
    }

    // increment variable
    var->value.data.number += loop->step;

    // check if loop should continue
    int continue_loop = 0;
    if (loop->step > 0) {
        continue_loop = (var->value.data.number <= loop->end);
    } else {
        continue_loop = (var->value.data.number >= loop->end);
    }

    if (continue_loop) {
        // jmp back to FOR line
        interp->current_line = loop->line_index + 1;
    } else {
        // pop FOR stack
        interp->for_stack_top--;
    }

    return 1;
}

int execute_line_tokens(Interpreter *interp, Token *tokens, int token_count, int start) {
    if (start >= token_count) return 1;

    Token *token = &tokens[start];

    if (token->type == TOKEN_COMMAND) {
        switch (token->command) {
            case CMD_PRINT:
                return execute_print(interp, tokens, token_count, start + 1);
            case CMD_LET:
                return execute_let(interp, tokens, token_count, start + 1);
            case CMD_INPUT:
                return execute_input(interp, tokens, token_count, start + 1);
            case CMD_IF:
                return execute_if(interp, tokens, token_count, start + 1);
            case CMD_FOR:
                return execute_for(interp, tokens, token_count, start + 1);
            case CMD_NEXT:
                return execute_next(interp);
            case CMD_GOTO: {
                if (start + 1 >= token_count || tokens[start + 1].type != TOKEN_NUMBER) {
                    print_error(interp, "GOTO requires line number");
                    return 0;
                }
                int line_num = (int)tokens[start + 1].value.data.number;
                int line_index = find_line_by_number(interp, line_num);
                if (line_index != -1) {
                    interp->current_line = line_index + 1;
                } else {
                    print_error(interp, "Line number not found");
                    return 0;
                }
                return 1;
            }
            case CMD_GOSUB: {
                if (start + 1 >= token_count || tokens[start + 1].type != TOKEN_NUMBER) {
                    print_error(interp, "GOSUB requires line number");
                    return 0;
                }

                if (interp->gosub_stack_top >= MAX_GOSUB_STACK - 1) {
                    print_error(interp, "GOSUB stack overflow");
                    return 0;
                }

                // push return address
                interp->gosub_stack[++interp->gosub_stack_top].return_line = interp->current_line;

                int line_num = (int)tokens[start + 1].value.data.number;
                int line_index = find_line_by_number(interp, line_num);
                if (line_index != -1) {
                    interp->current_line = line_index + 1;
                } else {
                    print_error(interp, "Line number not found");
                    return 0;
                }
                return 1;
            }
            case CMD_RETURN: {
                if (interp->gosub_stack_top < 0) {
                    print_error(interp, "RETURN without GOSUB");
                    return 0;
                }

                interp->current_line = interp->gosub_stack[interp->gosub_stack_top--].return_line;
                return 1;
            }
            case CMD_END:
            case CMD_STOP:
                interp->running = 0;
                return 1;
            case CMD_REM:
                return 1; // REM is a comment, do nothing
            default:
                print_error(interp, "Unknown command");
                return 0;
        }
    }
    if (token->type == TOKEN_VARIABLE) {
        // implicit LET
        return execute_let(interp, tokens, token_count, start);
    }

    print_error(interp, "Invalid statement");
    return 0;
}

int execute_line(Interpreter *interp, int line_index) {
    if (line_index < 0 || line_index >= interp->line_count) {
        return 0;
    }

    Line *line = &interp->lines[line_index];
    return execute_line_tokens(interp, line->tokens, line->token_count, 0);
}

int execute_program(Interpreter *interp) {
    interp->running = 1;
    interp->current_line = 0;

    while (interp->running && interp->current_line < interp->line_count) {
        if (!execute_line(interp, interp->current_line)) {
            return 0;
        }
        interp->current_line++;
    }

    return 1;
}

int load_program(Interpreter *interp, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return 0;
    }

    char line_buffer[MAX_LINE_LENGTH];
    while (fgets(line_buffer, sizeof(line_buffer), file)) {
        if (!parse_line(interp, line_buffer)) {
            fclose(file);
            return 0;
        }
    }

    fclose(file);

    // lines sorting
    for (int i = 0; i < interp->line_count - 1; i++) {
        for (int j = i + 1; j < interp->line_count; j++) {
            if (interp->lines[i].line_number > interp->lines[j].line_number) {
                Line temp = interp->lines[i];
                interp->lines[i] = interp->lines[j];
                interp->lines[j] = temp;
            }
        }
    }

    return 1;
}
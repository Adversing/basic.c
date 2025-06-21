#include "interpreter/basic_interpreter.h"
#include <time.h>

void print_usage() {
    printf("BASIC Interpreter Usage:\n");
    printf("  basic_interpreter <filename>    - Load and run BASIC program from file\n");
    printf("  basic_interpreter               - Interactive mode\n");
    printf("\nBASIC Syntax:\n");
    printf("  command                         - Execute immediately\n");
    printf("  line_number command             - Add to program (use RUN to execute)\n");
    printf("\nSupported BASIC Commands:\n");
    printf("  PRINT expr [, expr] [; expr]    - Print expressions\n");
    printf("  LET var = expr                  - Assign value to variable\n");
    printf("  INPUT [prompt;] var             - Input value to variable\n");
    printf("  IF condition THEN statement     - Conditional execution\n");
    printf("  FOR var = start TO end [STEP s] - For loop\n");
    printf("  NEXT [var]                      - End of for loop\n");
    printf("  GOTO line_number                - Jump to line\n");
    printf("  GOSUB line_number               - Call subroutine\n");
    printf("  RETURN                          - Return from subroutine\n");
    printf("  END                             - End program\n");
    printf("  REM comment                     - Comment line\n");
    printf("\nSupported Functions:\n");
    printf("  ABS(x), SIN(x), COS(x), TAN(x), SQR(x)\n");
    printf("  INT(x), RND(), LEN(s$), VAL(s$), STR$(x)\n");
    printf("  CHR$(x), ASC(s$)\n");
    printf("\nOperators: +, -, *, /, ^, MOD, =, <>, <, <=, >, >=, AND, OR, NOT\n");
    printf("\nSpecial Commands:\n");
    printf("  RUN                             - Run the loaded program\n");
    printf("  LIST                            - List the program lines\n");
    printf("  VARS                            - Show variables in memory\n");
    printf("  NEW                             - Clear program and variables\n");
    printf("  QUIT or EXIT                    - Exit the interpreter\n");
    printf("\nExamples:\n");
    printf("  LET A = 5                       - Execute immediately, A=5 in memory\n");
    printf("  10 LET B = 10                   - Add to program line 10\n");
    printf("  20 PRINT B                      - Add to program line 20\n");
    printf("  RUN                             - Execute program (creates B=10)\n");
}

void show_variables(Interpreter *interp) {
    if (!interp) return;
    
    if (interp->variable_count == 0) {
        printf("No variables defined\n");
        return;
    }
    
    printf("Defined variables:\n");
    for (int i = 0; i < interp->variable_count; i++) {
        Variable *var = &interp->variables[i];
        printf("  %s = ", var->name);
        if (var->value.type == VALUE_NUMBER) {
            printf("%.6g\n", var->value.data.number);
        } else if (var->value.type == VALUE_STRING && var->value.data.string) {
            printf("\"%s\"\n", var->value.data.string);
        } else {
            printf("(undefined)\n");
        }
    }
}

int is_valid_immediate_command(const char *trimmed) {
    if (!trimmed) return 0;
    
    int token_count;
    Token *tokens = tokenize(trimmed, &token_count);
    
    if (!tokens || token_count == 0) {
        return 0;
    }
    
    int valid = 0;
    if (tokens[0].type == TOKEN_COMMAND) {
        switch (tokens[0].command) {
            case CMD_PRINT:
            case CMD_LET:
            case CMD_INPUT:
                valid = 1;
                break;
            default:
                valid = 0;
                break;
        }
    } else if (tokens[0].type == TOKEN_VARIABLE) {
        valid = 1;
    }
    
    cleanup_tokens(tokens, token_count);
    return valid;
}

void interactive_mode() {
    Interpreter interp;
    init_interpreter(&interp);
    
    printf("BASIC Interpreter\n");
    printf("Type 'HELP' for commands, 'QUIT' to exit\n\n");
    
    char input[MAX_LINE_LENGTH];
    while (1) {
        printf("READY\n");
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }
        
        char *newline = strchr(input, '\n');
        if (newline) *newline = '\0';
        
        char *trimmed = input;
        while (isspace(*trimmed)) trimmed++;
        if (!*trimmed) continue;
        
        strcpy(interp.error_message, "");
        
        if (strcasecmp(trimmed, "QUIT") == 0 || strcasecmp(trimmed, "EXIT") == 0) {
            break;
        } else if (strcasecmp(trimmed, "HELP") == 0) {
            print_usage();
            continue;
        } else if (strcasecmp(trimmed, "RUN") == 0) {
            if (interp.line_count == 0) {
                printf("No program loaded. Use line numbers to add program lines.\n");
                continue;
            }

            printf("Running program...\n");
            if (!execute_program(&interp)) {
                if (strlen(interp.error_message) == 0) {
                    printf("Program execution failed\n");
                }
            }
            continue;
        } else if (strcasecmp(trimmed, "LIST") == 0) {
            if (interp.line_count == 0) {
                printf("No program loaded\n");
            } else {
                for (int i = 0; i < interp.line_count; i++) {
                    printf("%d %s\n", interp.lines[i].line_number, 
                           interp.lines[i].text ? interp.lines[i].text : "");
                }
            }
            continue;
        } else if (strcasecmp(trimmed, "VARS") == 0) {
            show_variables(&interp);
            continue;
        } else if (strcasecmp(trimmed, "NEW") == 0) {
            cleanup_interpreter(&interp);
            init_interpreter(&interp);
            printf("Program cleared\n");
            continue;
        } else if (strncasecmp(trimmed, "DEBUG ", 6) == 0) {
            char *expr = trimmed + 6;
            int token_count;
            Token *tokens = tokenize(expr, &token_count);
            if (tokens && token_count > 0) {
                printf("Tokenization of '%s':\n", expr);
                for (int i = 0; i < token_count; i++) {
                    printf("  [%d] Type: ", i);
                    switch (tokens[i].type) {
                        case TOKEN_NUMBER: printf("NUMBER"); break;
                        case TOKEN_STRING: printf("STRING"); break;
                        case TOKEN_VARIABLE: printf("VARIABLE"); break;
                        case TOKEN_COMMAND: printf("COMMAND"); break;
                        case TOKEN_OPERATOR: printf("OPERATOR"); break;
                        case TOKEN_FUNCTION: printf("FUNCTION"); break;
                        case TOKEN_DELIMITER: printf("DELIMITER"); break;
                        default: printf("UNKNOWN"); break;
                    }
                    printf(" Text: '%s'\n", tokens[i].text ? tokens[i].text : "NULL");
                }
                cleanup_tokens(tokens, token_count);
            } else {
                printf("Failed to tokenize expression\n");
            }
            continue;
        }
        
        if (!isdigit(trimmed[0])) {
            if (is_valid_immediate_command(trimmed)) {
                int token_count;
                Token *tokens = tokenize(trimmed, &token_count);
                if (tokens && token_count > 0) {
                    if (!execute_line_tokens(&interp, tokens, token_count, 0)) {
                        if (strlen(interp.error_message) == 0) {
                            printf("Error executing command\n");
                        }
                    }
                    cleanup_tokens(tokens, token_count);
                } else {
                    printf("Syntax error\n");
                }
            } else {
                printf("Unknown command: %s\n", trimmed);
                printf("Type 'HELP' for available commands\n");
            }
        } else {
            if (!parse_line(&interp, trimmed)) {
                if (strlen(interp.error_message) == 0) {
                    printf("Syntax error\n");
                }
                continue;
            }
            
            for (int i = 0; i < interp.line_count - 1; i++) {
                for (int j = i + 1; j < interp.line_count; j++) {
                    if (interp.lines[i].line_number > interp.lines[j].line_number) {
                        Line temp = interp.lines[i];
                        interp.lines[i] = interp.lines[j];
                        interp.lines[j] = temp;
                    }
                }
            }
        }
    }
    
    cleanup_interpreter(&interp);
}

int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));
    
    if (argc == 1) {
        interactive_mode();
        return 0;
    }
    
    if (argc != 2) {
        print_usage();
        return 1;
    }
    
    Interpreter interp;
    init_interpreter(&interp);
    
    printf("Loading BASIC program: %s\n", argv[1]);
    if (!load_program(&interp, argv[1])) {
        printf("Failed to load program\n");
        cleanup_interpreter(&interp);
        return 1;
    }
    
    printf("Program loaded successfully. %d lines.\n", interp.line_count);
    printf("Running program...\n\n");
    
    if (!execute_program(&interp)) {
        if (strlen(interp.error_message) == 0) {
            printf("Program execution failed\n");
        }
        cleanup_interpreter(&interp);
        return 1;
    }
    
    printf("\nProgram execution completed.\n");
    cleanup_interpreter(&interp);
    return 0;
}
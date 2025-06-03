#include "interpreter/basic_interpreter.h"

int get_precedence(const Operator op) {
    switch (op) {
        case OP_OR: return 1;
        case OP_AND: return 2;
        case OP_NOT: return 3;
        case OP_EQUAL:
        case OP_NOT_EQUAL:
        case OP_LESS:
        case OP_LESS_EQUAL:
        case OP_GREATER:
        case OP_GREATER_EQUAL: return 4;
        case OP_PLUS:
        case OP_MINUS: return 5;
        case OP_MULTIPLY:
        case OP_DIVIDE:
        case OP_MOD: return 6;
        case OP_POWER: return 7;
        default: return 0;
    }
}

Value apply_operator(Interpreter *interp, Value left, Operator op, Value right) {
    Value result = create_number_value(0);

    // string ops
    if (left.type == VALUE_STRING || right.type == VALUE_STRING) {
        if (op == OP_PLUS) {
            // "str1" + "str2"
            char *concat = malloc(strlen(left.data.string) + strlen(right.data.string) + 1);
            strcpy(concat, left.data.string);
            strcat(concat, right.data.string);
            result = create_string_value(concat);
            free(concat);
            return result;
        }
        if (op == OP_EQUAL) {
            // "str1" = "str2"
            result.data.number = strcmp(left.data.string, right.data.string) == 0 ? 1 : 0;
            return result;
        }
        print_error(interp, "Invalid string operation");
        return result;
    }

    // num ops
    const double l = left.data.number;
    const double r = right.data.number;

    switch (op) {
        case OP_PLUS: // a + b
            result.data.number = l + r;
            break;
        case OP_MINUS: // a - b
            result.data.number = l - r;
            break;
        case OP_MULTIPLY: // a * b
            result.data.number = l * r;
            break;
        case OP_DIVIDE: // a / b
            if (r == 0) {
                print_error(interp, "Division by zero");
                return result;
            }
            result.data.number = l / r;
            break;
        case OP_POWER: // a ^ b
            result.data.number = pow(l, r);
            break;
        case OP_MOD: // a MOD b
            if (r == 0) {
                print_error(interp, "Division by zero in MOD");
                return result;
            }
            result.data.number = fmod(l, r);
            break;
        case OP_EQUAL: // a = b
            result.data.number = (l == r) ? 1 : 0;
            break;
        case OP_NOT_EQUAL: // a <> b
            result.data.number = (l != r) ? 1 : 0;
            break;
        case OP_LESS: // a < b
            result.data.number = (l < r) ? 1 : 0;
            break;
        case OP_LESS_EQUAL: // a <= b
            result.data.number = (l <= r) ? 1 : 0;
            break;
        case OP_GREATER: // a > b
            result.data.number = (l > r) ? 1 : 0;
            break;
        case OP_GREATER_EQUAL: // a >= b
            result.data.number = (l >= r) ? 1 : 0;
            break;
        case OP_AND: // a AND b
            result.data.number = (l != 0 && r != 0) ? 1 : 0;
            break;
        case OP_OR: // a OR b
            result.data.number = (l != 0 || r != 0) ? 1 : 0;
            break;
        default:
            print_error(interp, "Unknown operator");
            break;
    }

    return result;
}

Value apply_function(Interpreter *interp, Function func, Value *args, int arg_count) {
    Value result = create_number_value(0);

    switch (func) {
        case FUNC_ABS:
            if (arg_count != 1 || args[0].type != VALUE_NUMBER) {
                print_error(interp, "ABS requires one numeric argument");
                return result;
            }
            result.data.number = fabs(args[0].data.number);
            break;

        case FUNC_SIN:
            if (arg_count != 1 || args[0].type != VALUE_NUMBER) {
                print_error(interp, "SIN requires one numeric argument");
                return result;
            }
            result.data.number = sin(args[0].data.number);
            break;

        case FUNC_COS:
            if (arg_count != 1 || args[0].type != VALUE_NUMBER) {
                print_error(interp, "COS requires one numeric argument");
                return result;
            }
            result.data.number = cos(args[0].data.number);
            break;

        case FUNC_TAN:
            if (arg_count != 1 || args[0].type != VALUE_NUMBER) {
                print_error(interp, "TAN requires one numeric argument");
                return result;
            }
            result.data.number = tan(args[0].data.number);
            break;

        case FUNC_SQR:
            if (arg_count != 1 || args[0].type != VALUE_NUMBER) {
                print_error(interp, "SQR requires one numeric argument");
                return result;
            }
            if (args[0].data.number < 0) {
                print_error(interp, "SQR of negative number");
                return result;
            }
            result.data.number = sqrt(args[0].data.number);
            break;

        case FUNC_INT:
            if (arg_count != 1 || args[0].type != VALUE_NUMBER) {
                print_error(interp, "INT requires one numeric argument");
                return result;
            }
            result.data.number = floor(args[0].data.number);
            break;

        case FUNC_RND:
            result.data.number = (double)rand() / RAND_MAX;
            break;

        case FUNC_LEN:
            if (arg_count != 1 || args[0].type != VALUE_STRING) {
                print_error(interp, "LEN requires one string argument");
                return result;
            }
            result.data.number = strlen(args[0].data.string);
            break;

        case FUNC_VAL:
            if (arg_count != 1 || args[0].type != VALUE_STRING) {
                print_error(interp, "VAL requires one string argument");
                return result;
            }
            result.data.number = atof(args[0].data.string);
            break;

        case FUNC_STR:
            if (arg_count != 1 || args[0].type != VALUE_NUMBER) {
                print_error(interp, "STR$ requires one numeric argument");
                return result;
            }
            char str_buffer[50];
            sprintf(str_buffer, "%.6g", args[0].data.number);
            result = create_string_value(str_buffer);
            break;

        case FUNC_CHR:
            if (arg_count != 1 || args[0].type != VALUE_NUMBER) {
                print_error(interp, "CHR$ requires one numeric argument");
                return result;
            }
            char chr_buffer[2];
            chr_buffer[0] = (char)args[0].data.number;
            chr_buffer[1] = '\0';
            result = create_string_value(chr_buffer);
            break;

        case FUNC_ASC:
            if (arg_count != 1 || args[0].type != VALUE_STRING) {
                print_error(interp, "ASC requires one string argument");
                return result;
            }
            if (strlen(args[0].data.string) == 0) {
                print_error(interp, "ASC of empty string");
                return result;
            }
            result.data.number = (double)args[0].data.string[0];
            break;

        default:
            print_error(interp, "Unknown function");
            break;
    }

    return result;
}

Value evaluate_expression(Interpreter *interp, Token *tokens, int start, int end) {
    Value result = create_number_value(0);

    if (start > end) {
        return result;
    }

    if (start == end) {
        const Token *token = &tokens[start];
        switch (token->type) {
            case TOKEN_NUMBER:
            case TOKEN_STRING:
                return token->value;
            case TOKEN_VARIABLE: {
                Variable *var = get_variable(interp, token->text);
                if (!var) {
                    print_error(interp, "Undefined variable");
                    return result;
                }
                return var->value;
            }
            default:
                print_error(interp, "Invalid expression");
                return result;
        }
    }

    int paren_level = 0;
    for (int i = start; i <= end; i++) {
        if (tokens[i].type == TOKEN_DELIMITER && strcmp(tokens[i].text, "(") == 0) {
            paren_level++;
        } else if (tokens[i].type == TOKEN_DELIMITER && strcmp(tokens[i].text, ")") == 0) {
            paren_level--;
            if (paren_level == 0 && i == end && start < end) {
                // expr is wrapped so this must be called recursively
                return evaluate_expression(interp, tokens, start + 1, end - 1);
            }
        }
    }

    // look for operator with the lowest precedence (rightmost)
    int op_pos = -1;
    int min_precedence = 999;
    paren_level = 0;

    for (int i = start; i <= end; i++) {
        if (tokens[i].type == TOKEN_DELIMITER && strcmp(tokens[i].text, "(") == 0) {
            paren_level++;
        } else if (tokens[i].type == TOKEN_DELIMITER && strcmp(tokens[i].text, ")") == 0) {
            paren_level--;
        } else if (paren_level == 0 && tokens[i].type == TOKEN_OPERATOR) {
            const int precedence = get_precedence(tokens[i].operator);
            if (precedence <= min_precedence) {
                min_precedence = precedence;
                op_pos = i;
            }
        }
    }

    if (op_pos != -1) {
        const Value left = evaluate_expression(interp, tokens, start, op_pos - 1);
        const Value right = evaluate_expression(interp, tokens, op_pos + 1, end);
        return apply_operator(interp, left, tokens[op_pos].operator, right);
    }

    // func calls block
    if (tokens[start].type == TOKEN_FUNCTION) {
        Function func = tokens[start].function;
        Value args[10];
        int arg_count = 0;

        // func(args) parsing
        if (start + 1 <= end && tokens[start + 1].type == TOKEN_DELIMITER &&
            strcmp(tokens[start + 1].text, "(") == 0) {
            int arg_start = start + 2;
            int arg_end = arg_start;
            paren_level = 0;

            for (int i = arg_start; i <= end; i++) {
                if (tokens[i].type == TOKEN_DELIMITER && strcmp(tokens[i].text, "(") == 0) {
                    paren_level++;
                } else if (tokens[i].type == TOKEN_DELIMITER && strcmp(tokens[i].text, ")") == 0) {
                    if (paren_level == 0) {
                        if (arg_end >= arg_start) {
                            args[arg_count++] = evaluate_expression(interp, tokens, arg_start, arg_end - 1);
                        }
                        break;
                    }
                    paren_level--;
                } else if (paren_level == 0 && tokens[i].type == TOKEN_DELIMITER &&
                          strcmp(tokens[i].text, ",") == 0) {
                    args[arg_count++] = evaluate_expression(interp, tokens, arg_start, arg_end - 1);
                    arg_start = i + 1;
                    arg_end = arg_start;
                    continue;
                }
                arg_end = i + 1;
            }
        }

        return apply_function(interp, func, args, arg_count);
    }

    print_error(interp, "Invalid expression");
    return result;
}
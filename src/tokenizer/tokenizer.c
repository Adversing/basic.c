#include "interpreter/basic_interpreter.h"

#define MAX_TOKENS 1000

Token *tokenize(const char *text, int *token_count) {
    if (!text || !token_count) {
        if (token_count) *token_count = 0;
        return NULL;
    }

    Token *tokens = malloc(sizeof(Token) * MAX_TOKENS);
    if (!tokens) {
        *token_count = 0;
        return NULL;
    }
    
    *token_count = 0;

    const char *ptr = text;
    while (*ptr && *token_count < MAX_TOKENS - 1) {
        while (isspace(*ptr)) ptr++;
        if (!*ptr) break;

        Token *token = &tokens[(*token_count)++];
        token->text = NULL;
        token->type = TOKEN_ERROR;
        token->value.type = VALUE_NUMBER;
        token->value.data.number = 0;
        token->command = CMD_UNKNOWN;
        token->operator = OP_UNKNOWN;
        token->function = FUNC_UNKNOWN;

        if (isdigit(*ptr) || (*ptr == '.' && isdigit(*(ptr + 1)))) { // numbers block
            const char *start = ptr;
            while (isdigit(*ptr) || *ptr == '.') ptr++;

            int len = ptr - start;
            if (len > 0) {
                token->text = malloc(len + 1);
                if (token->text) {
                    strncpy(token->text, start, len);
                    token->text[len] = '\0';

                    token->type = TOKEN_NUMBER;
                    token->value = create_number_value(atof(token->text));
                }
            }
        } else if (*ptr == '"') { // string literals
            ptr++; // skip opening quote
            
            char temp_buffer[MAX_LINE_LENGTH];
            int temp_len = 0;
            
            while (*ptr && *ptr != '"' && temp_len < MAX_LINE_LENGTH - 1) {
                if (*ptr == '\\' && *(ptr + 1)) {
                    switch (*(ptr + 1)) {
                        case 'n':
                            temp_buffer[temp_len++] = '\n';
                            ptr += 2;
                            break;
                        case 't':
                            temp_buffer[temp_len++] = '\t';
                            ptr += 2;
                            break;
                        case 'r':
                            temp_buffer[temp_len++] = '\r';
                            ptr += 2;
                            break;
                        case '\\':
                            temp_buffer[temp_len++] = '\\';
                            ptr += 2;
                            break;
                        case '"':
                            temp_buffer[temp_len++] = '"';
                            ptr += 2;
                            break;
                        case '\'':
                            temp_buffer[temp_len++] = '\'';
                            ptr += 2;
                            break;
                        default:
                            temp_buffer[temp_len++] = *ptr++;
                            break;
                    }
                } else {
                    temp_buffer[temp_len++] = *ptr++;
                }
            }

            if (*ptr != '"') {
                token->type = TOKEN_ERROR;
                (*token_count)--; 
                continue;
            }

            temp_buffer[temp_len] = '\0';
            
            token->text = malloc(temp_len + 1);
            if (token->text) {
                strcpy(token->text, temp_buffer);
                token->type = TOKEN_STRING;
                token->value = create_string_value(temp_buffer);
            }
            
            ptr++; // skip closing quote
        } else if (strncmp(ptr, "<=", 2) == 0 || strncmp(ptr, ">=", 2) == 0 ||
                 strncmp(ptr, "<>", 2) == 0) {
            token->text = malloc(3);
            if (token->text) {
                strncpy(token->text, ptr, 2);
                token->text[2] = '\0';
                token->type = TOKEN_OPERATOR;
                token->operator = get_operator(token->text);
            }
            ptr += 2;
        } else if (strchr("+-*/^=<>(),:;", *ptr)) { // single character operators and delimiters
            token->text = malloc(2);
            if (token->text) {
                token->text[0] = *ptr;
                token->text[1] = '\0';

                if (strchr("(),:;", *ptr)) {
                    token->type = TOKEN_DELIMITER;
                } else {
                    token->type = TOKEN_OPERATOR;
                    token->operator = get_operator(token->text);
                }
            }
            ptr++;
        } else if (isalpha(*ptr)) { // identifiers (commands, functions, variables, operators)
            const char *start = ptr;
            while (isalnum(*ptr) || *ptr == '$' || *ptr == '_') ptr++;

            int len = ptr - start;
            if (len > 0 && len < 32) { // limit identifier length
                token->text = malloc(len + 1);
                if (token->text) {
                    strncpy(token->text, start, len);
                    token->text[len] = '\0';

                    // check if it's a command first
                    const Command cmd = get_command(token->text);
                    if (cmd != CMD_UNKNOWN) {
                        token->type = TOKEN_COMMAND;
                        token->command = cmd;
                    } else {
                        // check if it's an operator (MOD, AND, OR, NOT)
                        const Operator op = get_operator(token->text);
                        if (op != OP_UNKNOWN) {
                            token->type = TOKEN_OPERATOR;
                            token->operator = op;
                        } else {
                            // check if it's a function
                            const Function func = get_function(token->text);
                            if (func != FUNC_UNKNOWN) {
                                token->type = TOKEN_FUNCTION;
                                token->function = func;
                            } else {
                                // default to variable
                                token->type = TOKEN_VARIABLE;
                            }
                        }
                    }
                }
            } else {
                // identifier too long, skip it
                (*token_count)--; 
                continue;
            }
        } else {
            // unknown character, skip it
            (*token_count)--; 
            ptr++;
        }
    }

    return tokens;
}

void cleanup_tokens(Token *tokens, int token_count) {
    if (!tokens || token_count <= 0) return;
    
    for (int i = 0; i < token_count; i++) {
        if (tokens[i].text) {
            free(tokens[i].text);
            tokens[i].text = NULL;
        }
        cleanup_value(&tokens[i].value);
    }
    
    free(tokens);
}
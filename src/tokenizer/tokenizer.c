#include "interpreter/basic_interpreter.h"

Token *tokenize(const char *text, int *token_count) {
    Token *tokens = malloc(sizeof(Token) * 100);
    *token_count = 0;

    const char *ptr = text;
    while (*ptr) {
        while (isspace(*ptr)) ptr++;
        if (!*ptr) break;

        Token *token = &tokens[(*token_count)++];
        token->text = NULL;
        token->type = TOKEN_ERROR;

        if (isdigit(*ptr) || (*ptr == '.' && isdigit(*(ptr + 1)))) { // numbers block
            const char *start = ptr;
            while (isdigit(*ptr) || *ptr == '.') ptr++;

            int len = ptr - start;
            token->text = malloc(len + 1);
            strncpy(token->text, start, len);
            token->text[len] = '\0';

            token->type = TOKEN_NUMBER;
            token->value = create_number_value(atof(token->text));
        } else if (*ptr == '"') { // char*
            ptr++; // ptr skips opening quote
            const char *start = ptr;
            while (*ptr && *ptr != '"') ptr++;

            if (*ptr != '"') {
                token->type = TOKEN_ERROR;
                continue;
            }

            int len = ptr - start;
            token->text = malloc(len + 1);
            strncpy(token->text, start, len);
            token->text[len] = '\0';

            token->type = TOKEN_STRING;
            token->value = create_string_value(token->text);
            ptr++; // ptr now skips the closing quote
        } else if (strncmp(ptr, "<=", 2) == 0 || strncmp(ptr, ">=", 2) == 0 ||
                 strncmp(ptr, "<>", 2) == 0) {
            token->text = malloc(3);
            strncpy(token->text, ptr, 2);
            token->text[2] = '\0';
            token->type = TOKEN_OPERATOR;
            token->operator = get_operator(token->text);
            ptr += 2;
        } else if (strchr("+-*/^=<>(),:;", *ptr)) { // single chars block
            token->text = malloc(2);
            token->text[0] = *ptr;
            token->text[1] = '\0';

            if (strchr("(),;:", *ptr)) {
                token->type = TOKEN_DELIMITER;
            } else {
                token->type = TOKEN_OPERATOR;
                token->operator = get_operator(token->text);
            }
            ptr++;
        } else if (isalpha(*ptr)) { // identifiers block
            const char *start = ptr;
            while (isalnum(*ptr) || *ptr == '$' || *ptr == '_') ptr++;

            int len = ptr - start;
            token->text = malloc(len + 1);
            strncpy(token->text, start, len);
            token->text[len] = '\0';

            // command check
            const Command cmd = get_command(token->text);
            if (cmd != CMD_UNKNOWN) {
                token->type = TOKEN_COMMAND;
                token->command = cmd;
            } else { // function check
                const Function func = get_function(token->text);
                if (func != FUNC_UNKNOWN) {
                    token->type = TOKEN_FUNCTION;
                    token->function = func;
                } else {
                    token->type = TOKEN_VARIABLE;
                }
            }
        } else { // unknown character
            ptr++;
        }
    }

    return tokens;
}
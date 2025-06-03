#include "basic_interpreter.h"

void init_interpreter(Interpreter *interp) {
    interp->line_count = 0;
    interp->variable_count = 0;
    interp->current_line = 0;
    interp->running = 0;
    interp->for_stack_top = -1;
    interp->gosub_stack_top = -1;
    interp->data_count = 0;
    interp->data_pointer = 0;
    strcpy(interp->error_message, "");
}

void cleanup_interpreter(Interpreter *interp) {
    for (int i = 0; i < interp->line_count; i++) {
        if (interp->lines[i].text) {
            free(interp->lines[i].text);
        }
        if (interp->lines[i].tokens) {
            for (int j = 0; j < interp->lines[i].token_count; j++) {
                if (interp->lines[i].tokens[j].text) {
                    free(interp->lines[i].tokens[j].text);
                }
                cleanup_value(&interp->lines[i].tokens[j].value);
            }
            free(interp->lines[i].tokens);
        }
    }

    for (int i = 0; i < interp->variable_count; i++) {
        cleanup_value(&interp->variables[i].value);
        if (interp->variables[i].is_array && interp->variables[i].array_data) {
            free(interp->variables[i].array_data);
            free(interp->variables[i].dim_sizes);
        }
    }

    for (int i = 0; i < interp->data_count; i++) {
        if (interp->data_values[i]) {
            free(interp->data_values[i]);
        }
    }
}

Value create_number_value(double number) {
    Value val;
    val.type = VALUE_NUMBER;
    val.data.number = number;
    return val;
}

Value create_string_value(const char *string) {
    Value val;
    val.type = VALUE_STRING;
    val.data.string = malloc(strlen(string) + 1);
    strcpy(val.data.string, string);
    return val;
}

void cleanup_value(Value *value) {
    if (value->type == VALUE_STRING && value->data.string) {
        free(value->data.string);
        value->data.string = NULL;
    }
}

Command get_command(const char *text) {
    if (strcasecmp(text, "PRINT") == 0) return CMD_PRINT;
    if (strcasecmp(text, "LET") == 0) return CMD_LET;
    if (strcasecmp(text, "INPUT") == 0) return CMD_INPUT;
    if (strcasecmp(text, "IF") == 0) return CMD_IF;
    if (strcasecmp(text, "THEN") == 0) return CMD_THEN;
    if (strcasecmp(text, "ELSE") == 0) return CMD_ELSE;
    if (strcasecmp(text, "GOTO") == 0) return CMD_GOTO;
    if (strcasecmp(text, "GOSUB") == 0) return CMD_GOSUB;
    if (strcasecmp(text, "RETURN") == 0) return CMD_RETURN;
    if (strcasecmp(text, "FOR") == 0) return CMD_FOR;
    if (strcasecmp(text, "TO") == 0) return CMD_TO;
    if (strcasecmp(text, "STEP") == 0) return CMD_STEP;
    if (strcasecmp(text, "NEXT") == 0) return CMD_NEXT;
    if (strcasecmp(text, "END") == 0) return CMD_END;
    if (strcasecmp(text, "REM") == 0) return CMD_REM;
    if (strcasecmp(text, "DATA") == 0) return CMD_DATA;
    if (strcasecmp(text, "READ") == 0) return CMD_READ;
    if (strcasecmp(text, "RESTORE") == 0) return CMD_RESTORE;
    if (strcasecmp(text, "DIM") == 0) return CMD_DIM;
    if (strcasecmp(text, "DEF") == 0) return CMD_DEF;
    if (strcasecmp(text, "ON") == 0) return CMD_ON;
    if (strcasecmp(text, "STOP") == 0) return CMD_STOP;
    if (strcasecmp(text, "RUN") == 0) return CMD_RUN;
    if (strcasecmp(text, "LIST") == 0) return CMD_LIST;
    if (strcasecmp(text, "NEW") == 0) return CMD_NEW;
    if (strcasecmp(text, "CLEAR") == 0) return CMD_CLEAR;
    return CMD_UNKNOWN;
}

Operator get_operator(const char *text) {
    if (strcmp(text, "+") == 0) return OP_PLUS;
    if (strcmp(text, "-") == 0) return OP_MINUS;
    if (strcmp(text, "*") == 0) return OP_MULTIPLY;
    if (strcmp(text, "/") == 0) return OP_DIVIDE;
    if (strcmp(text, "^") == 0) return OP_POWER;
    if (strcasecmp(text, "MOD") == 0) return OP_MOD;
    if (strcmp(text, "=") == 0) return OP_EQUAL;
    if (strcmp(text, "<>") == 0) return OP_NOT_EQUAL;
    if (strcmp(text, "<") == 0) return OP_LESS;
    if (strcmp(text, "<=") == 0) return OP_LESS_EQUAL;
    if (strcmp(text, ">") == 0) return OP_GREATER;
    if (strcmp(text, ">=") == 0) return OP_GREATER_EQUAL;
    if (strcasecmp(text, "AND") == 0) return OP_AND;
    if (strcasecmp(text, "OR") == 0) return OP_OR;
    if (strcasecmp(text, "NOT") == 0) return OP_NOT;
    return OP_UNKNOWN;
}

Function get_function(const char *text) {
    if (strcasecmp(text, "ABS") == 0) return FUNC_ABS;
    if (strcasecmp(text, "SIN") == 0) return FUNC_SIN;
    if (strcasecmp(text, "COS") == 0) return FUNC_COS;
    if (strcasecmp(text, "TAN") == 0) return FUNC_TAN;
    if (strcasecmp(text, "SQR") == 0) return FUNC_SQR;
    if (strcasecmp(text, "INT") == 0) return FUNC_INT;
    if (strcasecmp(text, "RND") == 0) return FUNC_RND;
    if (strcasecmp(text, "LEN") == 0) return FUNC_LEN;
    if (strcasecmp(text, "LEFT$") == 0) return FUNC_LEFT;
    if (strcasecmp(text, "RIGHT$") == 0) return FUNC_RIGHT;
    if (strcasecmp(text, "MID$") == 0) return FUNC_MID;
    if (strcasecmp(text, "VAL") == 0) return FUNC_VAL;
    if (strcasecmp(text, "STR$") == 0) return FUNC_STR;
    if (strcasecmp(text, "CHR$") == 0) return FUNC_CHR;
    if (strcasecmp(text, "ASC") == 0) return FUNC_ASC;
    return FUNC_UNKNOWN;
}

Variable *get_variable(Interpreter *interp, const char *name) {
    for (int i = 0; i < interp->variable_count; i++) {
        if (strcasecmp(interp->variables[i].name, name) == 0) {
            return &interp->variables[i];
        }
    }
    return NULL;
}

Variable *create_variable(Interpreter *interp, const char *name) {
    if (interp->variable_count >= MAX_VARIABLES) {
        print_error(interp, "Too many variables");
        return NULL;
    }

    Variable *var = &interp->variables[interp->variable_count++];
    strcpy(var->name, name);
    var->value = create_number_value(0);
    var->is_array = 0;
    var->dimensions = 0;
    var->dim_sizes = NULL;
    var->array_data = NULL;

    return var;
}

void set_variable(Interpreter *interp, const char *name, Value value) {
    Variable *var = get_variable(interp, name);
    if (!var) {
        var = create_variable(interp, name);
        if (!var) return;
    }

    cleanup_value(&var->value);
    var->value = value;
}

void print_error(Interpreter *interp, const char *message) {
    snprintf(interp->error_message, sizeof(interp->error_message),
             "Error at line %d: %s",
             interp->current_line > 0 ? interp->lines[interp->current_line - 1].line_number : 0,
             message);
    printf("%s\n", interp->error_message);
}

int find_line_by_number(Interpreter *interp, int line_number) {
    for (int i = 0; i < interp->line_count; i++) {
        if (interp->lines[i].line_number == line_number) {
            return i;
        }
    }
    return -1;
}
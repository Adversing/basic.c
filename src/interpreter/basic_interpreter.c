#include "basic_interpreter.h"

// lookup tables
typedef struct cl_t {
    const char *name;
    Command cmd;
} CommandLookup;

typedef struct ol_t {
    const char *name;
    Operator op;
} OperatorLookup;

typedef struct fl_t {
    const char *name;
    Function func;
} FunctionLookup;

static const CommandLookup command_table[] = {
    {"PRINT", CMD_PRINT},
    {"LET", CMD_LET},
    {"INPUT", CMD_INPUT},
    {"IF", CMD_IF},
    {"THEN", CMD_THEN},
    {"ELSE", CMD_ELSE},
    {"GOTO", CMD_GOTO},
    {"GOSUB", CMD_GOSUB},
    {"RETURN", CMD_RETURN},
    {"FOR", CMD_FOR},
    {"TO", CMD_TO},
    {"STEP", CMD_STEP},
    {"NEXT", CMD_NEXT},
    {"END", CMD_END},
    {"REM", CMD_REM},
    {"DATA", CMD_DATA},
    {"READ", CMD_READ},
    {"RESTORE", CMD_RESTORE},
    {"DIM", CMD_DIM},
    {"DEF", CMD_DEF},
    {"ON", CMD_ON},
    {"STOP", CMD_STOP},
    {"RUN", CMD_RUN},
    {"LIST", CMD_LIST},
    {"NEW", CMD_NEW},
    {"CLEAR", CMD_CLEAR},
    {NULL, CMD_UNKNOWN}
};

static const OperatorLookup operator_table[] = {
    {"+", OP_PLUS},
    {"-", OP_MINUS},
    {"*", OP_MULTIPLY},
    {"/", OP_DIVIDE},
    {"^", OP_POWER},
    {"MOD", OP_MOD},
    {"=", OP_EQUAL},
    {"<>", OP_NOT_EQUAL},
    {"<", OP_LESS},
    {"<=", OP_LESS_EQUAL},
    {">", OP_GREATER},
    {">=", OP_GREATER_EQUAL},
    {"AND", OP_AND},
    {"OR", OP_OR},
    {"NOT", OP_NOT},
    {NULL, OP_UNKNOWN}
};

static const FunctionLookup function_table[] = {
    {"ABS", FUNC_ABS},
    {"SIN", FUNC_SIN},
    {"COS", FUNC_COS},
    {"TAN", FUNC_TAN},
    {"SQR", FUNC_SQR},
    {"INT", FUNC_INT},
    {"RND", FUNC_RND},
    {"LEN", FUNC_LEN},
    {"LEFT$", FUNC_LEFT},
    {"RIGHT$", FUNC_RIGHT},
    {"MID$", FUNC_MID},
    {"VAL", FUNC_VAL},
    {"STR$", FUNC_STR},
    {"CHR$", FUNC_CHR},
    {"ASC", FUNC_ASC},
    {NULL, FUNC_UNKNOWN}
};

void init_interpreter(Interpreter *interp) {
    if (!interp) return;
    
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
    if (!interp) return;
    
    for (int i = 0; i < interp->line_count; i++) {
        if (interp->lines[i].text) {
            free(interp->lines[i].text);
            interp->lines[i].text = NULL;
        }
        if (interp->lines[i].tokens) {
            for (int j = 0; j < interp->lines[i].token_count; j++) {
                if (interp->lines[i].tokens[j].text) {
                    free(interp->lines[i].tokens[j].text);
                    interp->lines[i].tokens[j].text = NULL;
                }
                cleanup_value(&interp->lines[i].tokens[j].value);
            }
            free(interp->lines[i].tokens);
            interp->lines[i].tokens = NULL;
        }
        interp->lines[i].token_count = 0;
    }

    for (int i = 0; i < interp->variable_count; i++) {
        cleanup_value(&interp->variables[i].value);
        if (interp->variables[i].is_array && interp->variables[i].array_data) {
            for (int j = 0; j < interp->variables[i].dimensions; j++) {
                cleanup_value(&interp->variables[i].array_data[j]);
            }
            free(interp->variables[i].array_data);
            interp->variables[i].array_data = NULL;
            if (interp->variables[i].dim_sizes) {
                free(interp->variables[i].dim_sizes);
                interp->variables[i].dim_sizes = NULL;
            }
        }
    }

    for (int i = 0; i < interp->data_count; i++) {
        if (interp->data_values[i]) {
            free(interp->data_values[i]);
            interp->data_values[i] = NULL;
        }
    }
    
    interp->line_count = 0;
    interp->variable_count = 0;
    interp->data_count = 0;
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
    if (string && strlen(string) > 0) {
        val.data.string = malloc(strlen(string) + 1);
        if (val.data.string) {
            strcpy(val.data.string, string);
        } else {
            val.data.string = NULL;
        }
    } else {
        val.data.string = malloc(1);
        if (val.data.string) {
            val.data.string[0] = '\0';
        }
    }
    return val;
}

void cleanup_value(Value *value) {
    if (!value) return;
    
    if (value->type == VALUE_STRING && value->data.string) {
        free(value->data.string);
        value->data.string = NULL;
    }
    value->type = VALUE_NUMBER;
    value->data.number = 0;
}

Command get_command(const char *text) {
    if (!text) return CMD_UNKNOWN;
    
    for (int i = 0; command_table[i].name != NULL; i++) {
        if (strcasecmp(text, command_table[i].name) == 0) {
            return command_table[i].cmd;
        }
    }
    return CMD_UNKNOWN;
}

Operator get_operator(const char *text) {
    if (!text) return OP_UNKNOWN;
    
    for (int i = 0; operator_table[i].name != NULL; i++) {
        if (strcasecmp(text, operator_table[i].name) == 0) {
            return operator_table[i].op;
        }
    }
    return OP_UNKNOWN;
}

Function get_function(const char *text) {
    if (!text) return FUNC_UNKNOWN;
    
    for (int i = 0; function_table[i].name != NULL; i++) {
        if (strcasecmp(text, function_table[i].name) == 0) {
            return function_table[i].func;
        }
    }
    return FUNC_UNKNOWN;
}

Variable *get_variable(Interpreter *interp, const char *name) {
    if (!interp || !name) return NULL;
    
    for (int i = 0; i < interp->variable_count; i++) {
        if (strcasecmp(interp->variables[i].name, name) == 0) {
            return &interp->variables[i];
        }
    }
    return NULL;
}

Variable *create_variable(Interpreter *interp, const char *name) {
    if (!interp || !name) return NULL;
    
    if (interp->variable_count >= MAX_VARIABLES) {
        print_error(interp, "Too many variables");
        return NULL;
    }

    Variable *var = &interp->variables[interp->variable_count++];
    strncpy(var->name, name, sizeof(var->name) - 1);
    var->name[sizeof(var->name) - 1] = '\0';
    var->value = create_number_value(0);
    var->is_array = 0;
    var->dimensions = 0;
    var->dim_sizes = NULL;
    var->array_data = NULL;

    return var;
}

void set_variable(Interpreter *interp, const char *name, Value value) {
    if (!interp || !name) return;
    
    Variable *var = get_variable(interp, name);
    if (!var) {
        var = create_variable(interp, name);
        if (!var) return;
    }

    cleanup_value(&var->value);
    var->value = value;
}

void print_error(Interpreter *interp, const char *message) {
    if (!interp || !message) return;
    
    snprintf(interp->error_message, sizeof(interp->error_message),
             "Error at line %d: %s",
             interp->current_line > 0 ? interp->lines[interp->current_line - 1].line_number : 0,
             message);
    printf("%s\n", interp->error_message);
}

int find_line_by_number(Interpreter *interp, int line_number) {
    if (!interp) return -1;
    
    for (int i = 0; i < interp->line_count; i++) {
        if (interp->lines[i].line_number == line_number) {
            return i;
        }
    }
    return -1;
}
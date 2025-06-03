#ifndef BASIC_INTERPRETER_H
#define BASIC_INTERPRETER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_LINE_LENGTH 512
#define MAX_VARIABLES 1000
#define MAX_LINES 10000
#define MAX_GOTO_STACK 100
#define MAX_FOR_STACK 100
#define MAX_GOSUB_STACK 100
#define MAX_INPUT_LENGTH 256

typedef enum token_type_t {
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_VARIABLE,
    TOKEN_COMMAND,
    TOKEN_OPERATOR,
    TOKEN_FUNCTION,
    TOKEN_DELIMITER,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef enum command_t {
    CMD_PRINT,
    CMD_LET,
    CMD_INPUT,
    CMD_IF,
    CMD_THEN,
    CMD_ELSE,
    CMD_GOTO,
    CMD_GOSUB,
    CMD_RETURN,
    CMD_FOR,
    CMD_TO,
    CMD_STEP,
    CMD_NEXT,
    CMD_END,
    CMD_REM,
    CMD_DATA,
    CMD_READ,
    CMD_RESTORE,
    CMD_DIM,
    CMD_DEF,
    CMD_ON,
    CMD_STOP,
    CMD_RUN,
    CMD_LIST,
    CMD_NEW,
    CMD_CLEAR,
    CMD_UNKNOWN
} Command;

typedef enum operator_t {
    OP_PLUS,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_POWER,
    OP_MOD,
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_ASSIGN,
    OP_UNKNOWN
} Operator;

typedef enum function_t {
    FUNC_ABS,
    FUNC_SIN,
    FUNC_COS,
    FUNC_TAN,
    FUNC_SQR,
    FUNC_INT,
    FUNC_RND,
    FUNC_LEN,
    FUNC_LEFT,
    FUNC_RIGHT,
    FUNC_MID,
    FUNC_VAL,
    FUNC_STR,
    FUNC_CHR,
    FUNC_ASC,
    FUNC_UNKNOWN
} Function;

typedef enum value_type_t {
    VALUE_NUMBER,
    VALUE_STRING
} ValueType;

typedef struct value_t {
    ValueType type;
    union data_u {
        double number;
        char *string;
    } data;
} Value;

typedef struct variable_t {
    char name[32];
    Value value;
    int is_array;
    int dimensions;
    int *dim_sizes;
    Value *array_data;
} Variable;

typedef struct token_t {
    TokenType type;
    char *text;
    Value value;
    Command command;
    Operator operator;
    Function function;
} Token;

typedef struct line_t {
    int line_number;
    char *text;
    Token *tokens;
    int token_count;
} Line;

typedef struct for_stack_t {
    char variable[32];
    double start;
    double end;
    double step;
    int line_index;
} ForLoop;

typedef struct gosub_stack_t {
    int return_line;
} GosubStack;

typedef struct interpreter_t {
    Line lines[MAX_LINES];
    int line_count;
    Variable variables[MAX_VARIABLES];
    int variable_count;
    int current_line;
    int running;
    ForLoop for_stack[MAX_FOR_STACK];
    int for_stack_top;
    GosubStack gosub_stack[MAX_GOSUB_STACK];
    int gosub_stack_top;
    char *data_values[MAX_LINES];
    int data_count;
    int data_pointer;
    char error_message[256];
} Interpreter;

void init_interpreter(Interpreter *interp);
void cleanup_interpreter(Interpreter *interp);
int load_program(Interpreter *interp, const char *filename);
int parse_line(Interpreter *interp, const char *line_text);
int execute_program(Interpreter *interp);
int execute_line(Interpreter *interp, int line_index);
Token *tokenize(const char *text, int *token_count);
Value evaluate_expression(Interpreter *interp, Token *tokens, int start, int end);
Variable *get_variable(Interpreter *interp, const char *name);
Variable *create_variable(Interpreter *interp, const char *name);
void set_variable(Interpreter *interp, const char *name, Value value);
void print_error(Interpreter *interp, const char *message);
void cleanup_value(Value *value);
Value create_number_value(double number);
Value create_string_value(const char *string);
Command get_command(const char *text);
Operator get_operator(const char *text);
Function get_function(const char *text);
int find_line_by_number(Interpreter *interp, int line_number);
int execute_line_tokens(Interpreter *interp, Token *token, int token_count, int i);

#endif

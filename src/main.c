#include "interpreter/basic_interpreter.h"
#include <time.h>

void print_usage() {
    printf("BASIC Interpreter Usage:\n");
    printf("  basic_interpreter <filename>    - Load and run BASIC program from file\n");
    printf("  basic_interpreter               - Interactive mode\n");
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
    printf("  NEW                             - Clear the current program\n");
    printf("  QUIT or EXIT                    - Exit the interpreter\n");
}

void interactive_mode() {
    Interpreter interp;
    init_interpreter(&interp);
    
    printf("BASIC Interpreter\n");
    printf("Type 'HELP' for commands, 'QUIT' to exit\n\n");
    
    char input[MAX_LINE_LENGTH];
    while (1) {
        printf("READY\n");
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }
        
        char *newline = strchr(input, '\n');
        if (newline) *newline = '\0';
        
        char *trimmed = input;
        while (isspace(*trimmed)) trimmed++;
        if (!*trimmed) continue;
        
        if (strcasecmp(trimmed, "QUIT") == 0 || strcasecmp(trimmed, "EXIT") == 0) {
            break;
        } else if (strcasecmp(trimmed, "HELP") == 0) {
            print_usage();
            continue;
        } else if (strcasecmp(trimmed, "RUN") == 0) {
            if (interp.line_count == 0) {
                printf("No program loaded. Use 'NEW' to clear and start fresh.\n");
                continue;
            }

            printf("Running program...\n");
            if (!execute_program(&interp)) {
                printf("Program execution failed\n");
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
        } else if (strcasecmp(trimmed, "NEW") == 0) {
            cleanup_interpreter(&interp);
            init_interpreter(&interp);
            printf("Program cleared\n");
            continue;
        }
        
        if (!isdigit(trimmed[0])) {
            int token_count;
            Token *tokens = tokenize(trimmed, &token_count);
            if (tokens && token_count > 0) {
                if (!execute_line_tokens(&interp, tokens, token_count, 0)) {
                    printf("Error executing command\n");
                }
                
                for (int i = 0; i < token_count; i++) {
                    if (tokens[i].text) {
                        free(tokens[i].text);
                    }
                    cleanup_value(&tokens[i].value);
                }
                free(tokens);
            } else {
                printf("Syntax error\n");
            }
        } else {
            if (!parse_line(&interp, trimmed)) {
                printf("Syntax error\n");
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
    srand(time(NULL));
    
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
        printf("Program execution failed\n");
        cleanup_interpreter(&interp);
        return 1;
    }
    
    printf("\nProgram execution completed.\n");
    cleanup_interpreter(&interp);
    return 0;
}
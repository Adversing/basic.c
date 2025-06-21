# basic.c

A comprehensive (and small) BASIC language interpreter implementation in C that supports classic BASIC programming features and syntax.

## Features

### Core Language Support
- **Variables**: Numeric and string variables with automatic type detection
- **Data Types**: Numbers (double precision) and strings
- **Operators**: Arithmetic (`+`, `-`, `*`, `/`, `^`, `MOD`), comparison (`=`, `<>`, `<`, `<=`, `>`, `>=`), and logical (`AND`, `OR`, `NOT`)
- **String Operations**: Concatenation and comparison

### Control Flow
- **Conditional Statements**: `IF-THEN` with support for both statement execution and line jumps
- **Loops**: `FOR-NEXT` loops with optional `STEP` values (including negative steps)
- **Jumps**: `GOTO` for unconditional jumps to line numbers
- **Subroutines**: `GOSUB` and `RETURN` for subroutine calls

### Built-in Functions
- **Mathematical**: `ABS()`, `SIN()`, `COS()`, `TAN()`, `SQR()`, `INT()`, `RND()`
- **String Functions**: `LEN()`, `VAL()`, `STR$()`, `CHR$()`, `ASC()`

### I/O Operations
- **Output**: `PRINT` with support for expressions, separators (`,` for tabs, `;` for no separation)
- **Input**: `INPUT` with optional prompts for user data entry
- **Comments**: `REM` for program documentation

### Program Management
- **Line Numbers**: Traditional BASIC line numbering system
- **Program Loading**: Load and execute BASIC programs from files
- **Interactive Mode**: Direct command execution and program development

## Usage

### Command Line
```bash
# Run a BASIC program from file
basic.exe program.bas

# Start interactive mode
basic.exe
```

### Interactive Mode Commands
- `RUN` - Execute the loaded program
- `LIST` - Display program lines
- `VARS` - Show variables in memory
- `NEW` - Clear program and variables
- `HELP` - Show available commands
- `QUIT` or `EXIT` - Exit the interpreter

### BASIC Syntax Examples

#### Variables and Arithmetic
```basic
10 LET A = 10
20 LET B = 5
30 PRINT "A + B = "; A + B
40 PRINT "A ^ B = "; A ^ B
```

#### String Operations
```basic
10 LET NAME$ = "World"
20 PRINT "Hello " + NAME$ + "!\n"
30 PRINT "Length: "; LEN(NAME$)
```

#### Control Flow
```basic
10 FOR I = 1 TO 10 STEP 2
20   PRINT I; " ";
30 NEXT I
40 PRINT "\n"

50 IF A > B THEN PRINT "A is greater\n"
60 IF A < B THEN GOTO 100
```

#### Subroutines
```basic
10 GOSUB 1000
20 PRINT "Back from subroutine\n"
30 END

1000 PRINT "Inside subroutine\n"
1010 RETURN
```
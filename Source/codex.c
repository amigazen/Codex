/*
 * Codex - Amiga C Linter & Style Checker
 *
 * A lightweight code linter-cum-style checker for standard C code that checks 
 * for common programming issues and style violations on Amiga.
 *
 * Copyright (c) 2025 amigazen project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <utility/tagitem.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>
#include <clib/alib_protos.h>

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static const char *codex_verstag = "$VER: Codex 47.3 (26/12/2025)";
static const char *stack_cookie = "$STACK: 8192";
LONG oslibversion  = 47L; 

/* Configuration constants */
#define MAX_LINE_LENGTH 1024
#define MAX_FILENAME_LENGTH 256
#define MAX_ERRORS 1000
#define MAX_BLOCK_DEPTH 32 /* Max nesting depth for { } */

/* String parsing constants */
#define COMMENT_START_LENGTH 2
#define ESCAPE_SEQUENCE_LENGTH 2

/* Magic number detection constants */
#define PREVIOUS_CHAR_OFFSET 1
#define ARRAY_OFFSET_1 1
#define TRUNCATION_START 117
#define TRUNCATION_LENGTH 3
#define VERSION_START_OFFSET 1

/* Line excerpt constants */
#define LINE_EXCERPT_LIMIT 120

/* Buffer size constants */
#define REPLACEMENT_BUFFER_SIZE 64
#define LARGE_MESSAGE_BUFFER_SIZE 512

/* Amiga return codes - use different names to avoid conflicts */
#define CODEX_RETURN_OK 0
#define CODEX_RETURN_WARN 5
#define CODEX_RETURN_ERROR 20
#define CODEX_RETURN_FAIL 20

/* size_t is already defined by SAS/C in sys/commsize.h */

/* Error types */
typedef enum {
    ERROR_SYNTAX,
    ERROR_STYLE,
    ERROR_WARNING,
    ERROR_COMPILER,
    ERROR_COMMENT
} ErrorType;

/* Error structure */
typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    int line_number;
    int column;
    ErrorType type;
    char message[256];
    char line_excerpt[128];
} LintError;

/* State tracking structure */
typedef struct {
    int in_multiline_comment;
    int brace_depth;
    UBYTE statement_seen[MAX_BLOCK_DEPTH]; /* Flag for each brace depth */
    int forbid_active; /* Track if we're inside a Forbid() block */
    int forbid_line; /* Line number where Forbid() was called */
    int permit_line; /* Line number where Permit() was called */
    int forbid_count; /* Count of Forbid() calls */
    int permit_count; /* Count of Permit() calls */
} ParseState;

/* Global state */
static LintError errors[MAX_ERRORS];
static int error_count = 0;
static int total_lines = 0;
static int total_files = 0;
static ParseState parse_state;

/* Configuration flags */
static int enforce_amiga_pascalcase = 1;
static int enforce_compiler_compatibility = 1;
static int line_length_limit = 256;
static int quiet_mode = 0;

/* Validation mode flags */
static int validate_amiga_standards = 0;
static int validate_ndk_standards = 0;
static int validate_c89_standards = 1; /* Default enabled */
static int validate_c99_standards = 0;
static int validate_sasc_standards = 0;
static int validate_vbcc_standards = 0;
static int validate_dice_standards = 0;
static int validate_memsafe_standards = 0;

/* Compiler-specific keywords that need universal syntax - kept for future use */
/* static const char *compiler_specific_keywords[] = {
    "__saveds", "__save_ds", "__asm", "__reg", "__stdargs", "__far", "__interrupt", "__amigainterrupt", "__chip", "__fast"
};
static const char *universal_syntax[] = {
    "__SAVE_DS__", "__SAVE_DS__", "__ASM__", "__REG__", "__STDARGS__", "__FAR__", "__INTERRUPT__", "__INTERRUPT__", "__CHIP__", "__FAST__"
}; */

/* NDK compiler-specific.h reserved words that should use universal syntax */
/* This list should only contain non-universal keywords */
static const char *ndk_reserved_words[] = {
    "__saveds",
    "__save_ds",
    "__stkargs",
    "__amigainterrupt"
    /* Universal keywords like __asm, __reg, etc., have been removed */
};

/* --- Add these new arrays to codex.c --- */

/* Mapping of compiler-specific keywords to their universal equivalents */
static const char *non_universal_keywords[] = {
    "__saveds", "__save_ds", "__asm", "__reg", "__stdargs", "__far", "__interrupt",
    "__amigainterrupt", "__chip", "__fast", "__stkargs", "__attribute__", "__builtin_expect"
};
static const char *universal_replacements[] = {
    "__SAVE_DS__", "__SAVE_DS__", "__ASM__", "__REG__", "__STDARGS__", "__FAR__", "__INTERRUPT__",
    "__INTERRUPT__", "__CHIP__", "__FAST__", "__STDARGS__", "(none)", "(none)"
};

/* Universal syntax replacements for NDK words - kept for future use */
/* static const char *ndk_universal_syntax[] = {
    "__SAVE_DS__",
    "__ASM__", 
    "__REG__",
    "__STDARGS__",
    "__FAR__",
    "__INTERRUPT__",
    "__CHIP__",
    "__FAST__",
    "__NEAR__",
    "__HUGE__",
    "__SEG__",
    "__BASED__",
    "__PASCALL__",
    "__CDECL__",
    "__STDCALL__",
    "__FASTCALL__"
}; */

/* C89 keywords - kept for future use */
/* static const char *c89_keywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"
}; */

/* C99-specific keywords and features */
static const char *c99_keywords[] = {
    "inline", "restrict", "_Bool", "_Complex", "_Imaginary", "typeof"
};

/* C99-specific features to detect */
static const char *c99_features[] = {
    "//", "//*", "/*//", "//*/", /* C++ comments */
    "for (int ", "for (char ", "for (long ", /* Variable declarations in for loops */
    "struct { .", "struct { .x", "struct { .y", /* Designated initializers */
    "(int[]){", "(char[]){", "(struct Point){", /* Compound literals */
    "__VA_ARGS__", "..." /* Variadic macros */
};

/* C99 designated initializer patterns */
static const char *c99_designated_init_patterns[] = {
    "= { .", "= {.x", "= {.y", "= {.z", "= {.name", "= {.data",
    "= { .x", "= { .y", "= { .z", "= { .name", "= { .data",
    "= { .id", "= { .type", "= { .size", "= { .count", "= { .length",
    "= { .width", "= { .height", "= { .depth", "= { .flags", "= { .status"
};

/* C99 compound literal patterns */
static const char *c99_compound_literal_patterns[] = {
    "(int[]){", "(char[]){", "(long[]){", "(float[]){", "(double[])",
    "(unsigned int[]){", "(unsigned char[]){", "(unsigned long[])",
    "(struct ", "(union ", "(enum ", "){", "){", "){",
    "){", "){", "){", "){", "){", "){"
};

/* C99 variadic macro patterns */
static const char *c99_variadic_macro_patterns[] = {
    "__VA_ARGS__", "...", "##__VA_ARGS__", "__VA_OPT__",
    "#define", "##", "__VA_ARGS__", "__VA_OPT__"
};

/* C99 flexible array member patterns */
static const char *c99_flexible_array_patterns[] = {
    "char data[];", "int items[];", "long values[];", "float samples[];",
    "char name[];", "unsigned char buffer[];", "unsigned int flags[];",
    "short indices[];", "double measurements[];", "void *pointers[];"
};

/* C99+ standard library functions */
static const char *c99_stdlib_functions[] = {
    /* String functions */
    "snprintf", "vsnprintf", "strdup", "strndup", "strnlen", "strlcpy", "strlcat",
    "asprintf", "vasprintf", "open_memstream", "fmemopen", "getline", "getdelim",
    "strtok_r", "strerror_r", "memset_s", "strcpy_s", "strcat_s", "strncpy_s",
    "strncat_s", "strlen_s", "strcmp_s", "strncmp_s", "strchr_s", "strrchr_s",
    "strstr_s", "strpbrk_s", "strspn_s", "strcspn_s", "strtok_s",
    
    /* Math functions from <math.h> */
    "round", "lround", "llround", "trunc", "remainder", "fma", "nan",
    
    /* Integer conversion functions from <stdlib.h> */
    "atoll", "strtof", "strtold", "llabs",
    
    /* Greatest-width integer conversion from <inttypes.h> */
    "strtoimax", "strtoumax"
};

/* C89 standard library headers */
static const char *c89_header_files[] = {
    "<stdio.h>", "<stdlib.h>", "<string.h>", "<ctype.h>", "<math.h>",
    "<time.h>", "<locale.h>", "<setjmp.h>", "<signal.h>", "<errno.h>",
    "<assert.h>", "<limits.h>", "<float.h>"
};

/* C99+ header files */
static const char *c99_header_files[] = {
    "<stdint.h>", "<stdbool.h>", "<complex.h>", "<tgmath.h>", "<fenv.h>",
    "<inttypes.h>", "<wchar.h>", "<wctype.h>", "<uchar.h>", "<threads.h>",
    "<stdatomic.h>", "<stdnoreturn.h>", "<stdalign.h>", "<stdbit.h>"
};

/* Common stdlib functions (lowercase) */
static const char *stdlib_functions[] = {
    "printf", "scanf", "malloc", "free", "strcpy", "strlen", "fopen", "fclose", "fgets",
    "fputs", "fread", "fwrite", "fseek", "ftell", "rewind", "feof", "ferror", "clearerr",
    "strcat", "strcmp", "strncmp", "strncpy", "strncat", "strchr", "strrchr", "strstr",
    "strtok", "strerror", "strdup", "strndup", "strnlen", "strlcpy", "strlcat",
    "sprintf", "vsprintf", "snprintf", "vsnprintf", "sscanf", "fscanf",
    "calloc", "realloc", "memcpy", "memmove", "memcmp", "memset", "memchr",
    "abs", "labs", "llabs", "div", "ldiv", "lldiv", "rand", "srand",
    "atoi", "atol", "atoll", "strtol", "strtoul", "strtoll", "strtoull",
    "exit", "abort", "atexit", "system", "getenv", "setenv", "unsetenv",
    "time", "ctime", "gmtime", "localtime", "mktime", "strftime", "asctime",
    "isalpha", "isdigit", "isalnum", "isspace", "isupper", "islower", "toupper", "tolower",
    "sin", "cos", "tan", "asin", "acos", "atan", "atan2", "sinh", "cosh", "tanh",
    "exp", "log", "log10", "pow", "sqrt", "ceil", "floor", "fabs", "fmod",
    "setjmp", "longjmp", "signal", "raise", "qsort", "bsearch"
};

/* Common Amiga functions (PascalCase) */
static const char *amiga_functions[] = {
    "OpenLibrary", "CloseLibrary", "AllocMem", "FreeMem", "CreateMsgPort", "DeleteMsgPort",
    "DoIO", "OpenDevice", "CloseDevice", "ReadArgs", "Open", "Close", "Read", "Write" /* ... and others */
};

/* Memory-unsafe C standard library functions */
static const char *memsafe_unsafe_functions[] = {
    /* Buffer overflow prone functions */
    "strcpy", "strcat", "sprintf", "gets", "scanf", "fscanf", "sscanf",
    "strtok", "strerror", "tmpnam", "mktemp", "realpath", "vsprintf",
    
    /* Poor error handling functions */
    "atoi", "atol", "atof",
    
    /* Thread-unsafe functions */
    "getenv"
};

/* Memory-safe replacements for unsafe functions */
static const char *memsafe_safe_replacements[] = {
    /* Buffer overflow prone functions */
    "strncpy", "strncat", "snprintf", "fgets", "check_return_and_width", "check_return_and_width", "check_return_and_width",
    "strtok_r", "strerror_r", "tmpnam_r", "mkstemp", "realpath", "vsnprintf",
    
    /* Poor error handling functions */
    "strtol", "strtol", "strtod",
    
    /* Thread-unsafe functions */
    "getenv_s or use mutex protection"
};

/* Keywords that are incompatible with SAS/C (DICE, GCC, VBCC specific) */
static const char *sasc_keywords[] = {
    "__amigainterrupt",  /* VBCC-specific */
    "__stkargs",         /* DICE-specific */
    "__attribute__",     /* GCC-specific */
    "__builtin_",        /* GCC-specific */
    "__volatile__",      /* GCC-specific */
    "__const__",         /* GCC-specific */
    "__restrict__"       /* GCC-specific */
};

/* Keywords that are incompatible with VBCC (DICE, GCC, SAS/C specific) */
static const char *vbcc_keywords[] = {
    "__saveds",          /* SAS/C-specific */
    "__save_ds",         /* SAS/C-specific */
    "__stkargs",         /* DICE-specific */
    "__attribute__",     /* GCC-specific */
    "__builtin_",        /* GCC-specific */
    "__volatile__",      /* GCC-specific */
    "__const__",         /* GCC-specific */
    "__restrict__"       /* GCC-specific */
};

/* Function Prototypes - All functions must be declared before use */
static void add_error_with_excerpt(const char *filename, int line, int col, ErrorType type, const char *msg, const char *line_text);
static void add_error(const char *filename, int line, int col, ErrorType type, const char *msg);
static void add_codex_comment(const char *filename, int line, const char *comment);
static void process_line(const char *line, int line_num, const char *filename);
static void print_errors(void);
static void print_usage(void);
static int process_file(const char *filename);
/* static void trim_leading_whitespace(char *str); */
static char* find_first_non_whitespace(char *str);
static int is_declaration_keyword(const char *word);

/* Validation function prototypes */
static void check_amiga_standards(const char *line, int line_num, const char *filename, const char *original_line);
static void check_ndk_standards(const char *line, int line_num, const char *filename, const char *original_line);
static void check_c89_standards(const char *line, int line_num, const char *filename, const char *original_line);
static void check_c99_standards(const char *line, int line_num, const char *filename, const char *original_line);
static void check_sasc_standards(const char *line, int line_num, const char *filename, const char *original_line);
static void check_vbcc_standards(const char *line, int line_num, const char *filename, const char *original_line);
static void check_dice_standards(const char *line, int line_num, const char *filename, const char *original_line);
static void check_memsafe_standards(const char *line, int line_num, const char *filename, const char *original_line);
static void check_for_magic_numbers(const char *line, int line_num, const char *filename, const char *original_line);
static void check_forbid_permit_pairs(const char *line, int line_num, const char *filename, const char *original_line);
static void validate_forbid_permit_pairs(const char *filename);
static int is_ndk_reserved_word(const char *word);
static int is_c99_keyword(const char *word);
static int is_c99_feature(const char *line);
static int is_c99_designated_init(const char *line);
static int is_c99_compound_literal(const char *line);
static int is_c99_variadic_macro(const char *line);
static int is_c99_flexible_array(const char *line);
static int is_c99_stdlib_function(const char *line);
static int is_c89_header_file(const char *line);
static int is_c99_header_file(const char *line);
static int is_sasc_keyword(const char *word);
static int is_vbcc_keyword(const char *word);
static int is_memsafe_unsafe_function(const char *word);
static int is_amiga_function(const char *word);
static int find_memsafe_replacement(const char *function, char *replacement, size_t max_len);
static int find_universal_replacement(const char *keyword, char *replacement, size_t max_len);
static int is_stdlib_function(const char *word);

/* String function prototypes for Amiga compatibility - removed, using standard library */

/* Main entry point for Amiga CLI tools */
int main(int argc, char **argv) {
    int exit_code = CODEX_RETURN_OK;
    struct RDArgs *rda;
    STRPTR *current_file;
    int modes_shown = 0;
    static CONST_STRPTR template = "FILES/M/A,AMIGA/S,NDK/S,C89/S,C99/S,SASC/S,VBCC/S,DICE/S,MEMSAFE/S,QUIET/S,HELP/S";
    
    /* Using a struct for cleaner argument handling */
    struct {
        STRPTR *files;
        LONG amiga_standards;
        LONG ndk_standards;
        LONG c89_standards;
        LONG c99_standards;
        LONG sasc_standards;
        LONG vbcc_standards;
        LONG dice_standards;
        LONG memsafe_standards;
        LONG quiet;
        LONG help;
    } args = {0};

    rda = ReadArgs(template, (LONG *)&args, NULL);
    if (!rda) {
        Printf("Error: Invalid command line arguments\n");
        print_usage();
        return CODEX_RETURN_FAIL;
    }

    if (args.help) {
        print_usage();
        FreeArgs(rda);
        return CODEX_RETURN_OK;
    }

    /* Set configuration flags based on arguments */
    if (args.quiet) quiet_mode = 1;

    /* Set validation mode flags based on arguments */
    if (args.amiga_standards) validate_amiga_standards = 1;
    if (args.ndk_standards) validate_ndk_standards = 1;
    if (args.c89_standards) validate_c89_standards = 1;
    if (args.c99_standards) validate_c99_standards = 1;
    if (args.sasc_standards) validate_sasc_standards = 1;
    if (args.vbcc_standards) validate_vbcc_standards = 1;
    if (args.dice_standards) validate_dice_standards = 1;
    if (args.memsafe_standards) validate_memsafe_standards = 1;

    /* Implement mode dependencies with warnings for conflicts */
    if (validate_sasc_standards) {
        if (validate_c99_standards) {
            if (!quiet_mode) Printf("Warning: SAS/C mode overrides C99 mode (SAS/C is C89-only)\n");
        }
        validate_c89_standards = 1;  /* SASC implies C89 */
        validate_c99_standards = 0;  /* SASC does NOT imply C99 */
        enforce_compiler_compatibility = 1;  /* SASC enables compiler compatibility checking */
    }
    if (validate_vbcc_standards) {
        if (validate_c89_standards) {
            if (!quiet_mode) Printf("Warning: VBCC mode overrides C89 mode (VBCC supports C99)\n");
        }
        validate_c99_standards = 1;  /* VBCC implies C99 */
        validate_c89_standards = 0;  /* VBCC does NOT imply C89 */
        enforce_compiler_compatibility = 1;  /* VBCC enables compiler compatibility checking */
    }
    if (validate_amiga_standards) {
        if (!validate_ndk_standards) {
            if (!quiet_mode) Printf("Info: Amiga mode enables NDK validation\n");
        }
        validate_ndk_standards = 1;  /* AMIGA implies NDK */
        enforce_amiga_pascalcase = 1;  /* AMIGA enables PascalCase enforcement */
        enforce_compiler_compatibility = 1;  /* AMIGA enables compiler compatibility checking */
    }
    if (validate_dice_standards) {
        if (!validate_c89_standards) {
            if (!quiet_mode) Printf("Info: DICE mode enables C89 validation\n");
        }
        if (!validate_ndk_standards) {
            if (!quiet_mode) Printf("Info: DICE mode enables NDK validation\n");
        }
        validate_c89_standards = 1;  /* DICE implies C89 for now */
        validate_ndk_standards = 1;  /* DICE implies NDK */
        enforce_compiler_compatibility = 1;  /* DICE enables compiler compatibility checking */
    }
    if (validate_ndk_standards) {
        enforce_compiler_compatibility = 1;  /* NDK enables compiler compatibility checking */
    }
    if (validate_memsafe_standards) {
        if (!validate_c89_standards) {
            if (!quiet_mode) Printf("Info: MEMSAFE mode enables C89 validation\n");
        }
        validate_c89_standards = 1;  /* MEMSAFE implies C89 */
    }
    
    /* Ensure at least one standard is enabled - but don't override explicit mode choices */
    if (!validate_c89_standards && !validate_c99_standards) {
        /* Only default to C89 if no compiler mode was specified that would imply a standard */
        if (!validate_sasc_standards && !validate_vbcc_standards && !validate_dice_standards) {
            validate_c89_standards = 1;  /* Default to C89 if no specific mode specified */
        }
    }

    /* Correctly process multiple files from FILES/M */
    if (args.files) {
        current_file = args.files;
        while (*current_file) {
            if (process_file(*current_file) != 0) {
                exit_code = CODEX_RETURN_ERROR;
            }
            current_file++;
        }
    } else {
        if (!quiet_mode) Printf("No input files specified.\n");
        print_usage();
    }

    if (!quiet_mode) {
        Printf("\nCodex analysis complete.\n");
        
        /* Show active validation modes */
        Printf("Active validation modes: ");
        if (validate_amiga_standards) { Printf("%sAmiga", modes_shown++ ? ", " : ""); }
        if (validate_ndk_standards) { Printf("%sNDK", modes_shown++ ? ", " : ""); }
        if (validate_c89_standards) { Printf("%sC89", modes_shown++ ? ", " : ""); }
        if (validate_c99_standards) { Printf("%sC99", modes_shown++ ? ", " : ""); }
        if (validate_sasc_standards) { Printf("%sSAS/C", modes_shown++ ? ", " : ""); }
        if (validate_vbcc_standards) { Printf("%sVBCC", modes_shown++ ? ", " : ""); }
        if (validate_dice_standards) { Printf("%sDICE", modes_shown++ ? ", " : ""); }
        if (validate_memsafe_standards) { Printf("%sMEMSAFE", modes_shown++ ? ", " : ""); }
        if (modes_shown == 0) { Printf("None (basic style checking only)"); }
        Printf("\n");
        
        if (error_count > 0) {
            Printf("Found %ld issues in %ld files (%ld lines processed).\n", error_count, total_files, total_lines);
            print_errors();
            exit_code = CODEX_RETURN_WARN;
        } else {
            Printf("No issues found in %ld files (%ld lines processed).\n", total_files, total_lines);
        }
    } else {
        /* In quiet mode, only show errors, no summary */
        if (error_count > 0) {
            print_errors();
            exit_code = CODEX_RETURN_WARN;
        }
    }

    FreeArgs(rda);
    return exit_code;
}

/* Adds an error with line excerpt to the global error list */
static void add_error_with_excerpt(const char *filename, int line, int col, ErrorType type, const char *msg, const char *line_text) {
    if (error_count >= MAX_ERRORS) {
        if (error_count == MAX_ERRORS) { /* Print only once */
             Printf("Warning: Maximum error count reached. Further errors will be ignored.\n");
             error_count++;
        }
        return;
    }

    strncpy(errors[error_count].filename, filename, MAX_FILENAME_LENGTH - 1);
    errors[error_count].filename[MAX_FILENAME_LENGTH - 1] = '\0';
    errors[error_count].line_number = line;
    errors[error_count].column = col;
    errors[error_count].type = type;
    strncpy(errors[error_count].message, msg, sizeof(errors[error_count].message) - 1);
    errors[error_count].message[sizeof(errors[error_count].message) - 1] = '\0';
    
        /* Capture line excerpt (first LINE_EXCERPT_LIMIT chars to leave room for truncation indicator) */
    if (line_text && *line_text) {
        strncpy(errors[error_count].line_excerpt, line_text, LINE_EXCERPT_LIMIT);
        errors[error_count].line_excerpt[LINE_EXCERPT_LIMIT] = '\0';
        /* Add truncation indicator if line was too long */
        if (strlen(line_text) > LINE_EXCERPT_LIMIT) {
            strncpy(errors[error_count].line_excerpt + TRUNCATION_START, "...", TRUNCATION_LENGTH);
            errors[error_count].line_excerpt[LINE_EXCERPT_LIMIT] = '\0';
        }
    } else {
        errors[error_count].line_excerpt[0] = '\0';
    }

    error_count++;
}

/* Adds an error to the global error list (without excerpt) */
static void add_error(const char *filename, int line, int col, ErrorType type, const char *msg) {
    add_error_with_excerpt(filename, line, col, type, msg, NULL);
}

/* Adds a $CODEX: comment as a special error message for testing */
static void add_codex_comment(const char *filename, int line, const char *comment) {
    char message[256];
    
    if (error_count >= MAX_ERRORS) {
        if (error_count == MAX_ERRORS) { /* Print only once */
             Printf("Warning: Maximum error count reached. Further errors will be ignored.\n");
             error_count++;
        }
        return;
    }

    strncpy(errors[error_count].filename, filename, MAX_FILENAME_LENGTH - 1);
    errors[error_count].filename[MAX_FILENAME_LENGTH - 1] = '\0';
    errors[error_count].line_number = line;
    errors[error_count].column = 1;
    errors[error_count].type = ERROR_COMMENT; /* Use comment type for $CODEX: comments */
    
    /* Format the comment message */
    strncpy(message, comment, sizeof(message) - 1);
    message[sizeof(message) - 1] = '\0';
    strncpy(errors[error_count].message, message, sizeof(errors[error_count].message) - 1);
    errors[error_count].message[sizeof(errors[error_count].message) - 1] = '\0';

    error_count++;
}

/* A helper to check if a word is a C89 type or storage class keyword */
static int is_declaration_keyword(const char *word) {
    const char *decl_keywords[] = {
        "auto", "char", "const", "double", "enum", "extern", "float", "int", "long",
        "register", "short", "signed", "static", "struct", "typedef", "union",
        "unsigned", "void", "volatile"
    };
    int i;
    int num_keywords = sizeof(decl_keywords) / sizeof(decl_keywords[0]);
    for (i = 0; i < num_keywords; i++) {
        if (strcmp(word, decl_keywords[i]) == 0) return 1;
    }
    return 0;
}

/* Checks for all issues on a single line */
static void process_line(const char *line, int line_num, const char *filename) {
    char clean_line[MAX_LINE_LENGTH];
    char original_line[MAX_LINE_LENGTH];
    char *p;
    int in_string = 0;
    int in_char_literal = 0;
    const char *s;
    char *trimmed_line;
    char *first_word;
    char line_copy[MAX_LINE_LENGTH];
    char clean_comment[256];
    size_t comment_len;
    int initial_error_count = error_count; /* Store the error count at the start */

    strncpy(original_line, line, sizeof(original_line) - 1);
    original_line[sizeof(original_line) - 1] = '\0';
    
    /* Create a working copy */
    p = clean_line;
    s = line;

    /* Simple state machine to handle comments and string literals */
    
    while(*s) {
        if (parse_state.in_multiline_comment) {
            if (*s == '*' && *(s+1) == '/') {
                parse_state.in_multiline_comment = 0;
                s += COMMENT_START_LENGTH;
                continue;
            }
            s++;
            continue;
        }

        if (*s == '/' && *(s+1) == '*') {
            parse_state.in_multiline_comment = 1;
            s += COMMENT_START_LENGTH;
            continue;
        }

        if (*s == '/' && *(s+1) == '/') {
            /* Only flag C++ comments if C89 mode is active and SAS/C mode is not active (SAS/C supports them) */
            if (validate_c89_standards && !validate_sasc_standards) {
                add_error_with_excerpt(filename, line_num, s - line + ARRAY_OFFSET_1, ERROR_SYNTAX, "C++ comments ('//') are not allowed in C89.", original_line);
                if (error_count > initial_error_count) return; /* Exit after first error */
            }
            break; /* Rest of the line is a comment */
        }
        
        /* Basic string literal handling with proper escape sequence support */
        if (in_string || in_char_literal) {
            if (*s == '\\' && *(s+1) != '\0') {
                /* Skip over escaped characters */
                *p++ = *s++;
                *p++ = *s++;
                continue;
            }
        }
        
        /* Now check for quote characters (escapes already handled above) */
        if (*s == '"') in_string = !in_string;
        if (*s == '\'') in_char_literal = !in_char_literal;

        *p++ = *s++;
    }
    *p = '\0';

    /* After cleaning comments, check content */
    trimmed_line = find_first_non_whitespace(clean_line);
    if (!*trimmed_line) return; /* Line is empty or only comments */

    /* Check for $CODEX: comments ONLY if no other error has been found yet */
    if (error_count == initial_error_count) {
        const char *codex_pos = strstr(original_line, "$CODEX:");
        if (codex_pos) {
            const char *comment_start = codex_pos + 7; /* Skip "$CODEX:" */
            while (*comment_start == ' ' || *comment_start == '\t') comment_start++; /* Skip leading whitespace */
            if (*comment_start) {
                /* Find the end of the comment (before any trailing comment markers) */
                const char *comment_end = comment_start;
                while (*comment_end && *comment_end != '/' && *comment_end != '*') comment_end++;
                
                /* Create a clean comment message */
                comment_len = comment_end - comment_start;
                if (comment_len >= sizeof(clean_comment)) comment_len = sizeof(clean_comment) - 1;
                
                strncpy(clean_comment, comment_start, comment_len);
                clean_comment[comment_len] = '\0';
                
                /* Add the $CODEX comment as a test output since no other errors were found */
                add_codex_comment(filename, line_num, clean_comment);
            }
        }
    }

    /* --- STANDARDS VALIDATION CHECKS --- */
    if (validate_c89_standards) {
        check_c89_standards(clean_line, line_num, filename, original_line);
        if (error_count > initial_error_count) return; /* Exit after first error */
    }
    
    if (validate_c99_standards) {
        check_c99_standards(clean_line, line_num, filename, original_line);
        if (error_count > initial_error_count) return; /* Exit after first error */
    }
    
    if (validate_amiga_standards) {
        check_amiga_standards(clean_line, line_num, filename, original_line);
        if (error_count > initial_error_count) return; /* Exit after first error */
    }
    
    if (validate_ndk_standards) {
        check_ndk_standards(clean_line, line_num, filename, original_line);
        if (error_count > initial_error_count) return; /* Exit after first error */
    }
    
    if (validate_sasc_standards) {
        check_sasc_standards(clean_line, line_num, filename, original_line);
        if (error_count > initial_error_count) return; /* Exit after first error */
    }
    
    if (validate_vbcc_standards) {
        check_vbcc_standards(clean_line, line_num, filename, original_line);
        if (error_count > initial_error_count) return; /* Exit after first error */
    }
    
    if (validate_dice_standards) {
        check_dice_standards(clean_line, line_num, filename, original_line);
        if (error_count > initial_error_count) return; /* Exit after first error */
    }
    
    if (validate_memsafe_standards) {
        check_memsafe_standards(clean_line, line_num, filename, original_line);
        if (error_count > initial_error_count) return; /* Exit after first error */
    }

    /* --- MAGIC NUMBER CHECK --- */
    check_for_magic_numbers(clean_line, line_num, filename, original_line);
    if (error_count > initial_error_count) return; /* Exit after first error */

    /* --- FORBID/PERMIT PAIR CHECK --- */
    check_forbid_permit_pairs(clean_line, line_num, filename, original_line);
    if (error_count > initial_error_count) return; /* Exit after first error */

    /* --- C89 VARIABLE DECLARATION PLACEMENT --- */
    if (validate_c89_standards) {
        /* Use a more robust approach to avoid false positives with function pointers and complex declarations */
        char *line_copy = malloc(strlen(trimmed_line) + 1);
        if (line_copy) {
            strcpy(line_copy, trimmed_line);
            first_word = strtok(line_copy, " \t\n\r");

            if (first_word) {
                if (is_declaration_keyword(first_word)) {
                    /* Check if this is a simple variable declaration (not a function pointer or complex type) */
                    char *paren_pos = strchr(trimmed_line, '(');
                    char *brace_pos = strchr(trimmed_line, '{');
                    char *semicolon_pos = strchr(trimmed_line, ';');
                    
                    /* Only flag if it's a simple declaration (ends with semicolon, no parentheses before semicolon) */
                    if (semicolon_pos && (!paren_pos || semicolon_pos < paren_pos)) {
                        if (parse_state.brace_depth > 0 && parse_state.statement_seen[parse_state.brace_depth]) {
                            add_error_with_excerpt(filename, line_num, (trimmed_line - clean_line) + ARRAY_OFFSET_1, ERROR_SYNTAX, "Variable declaration after a statement is not allowed in C89.", original_line);
                            free(line_copy);
                            if (error_count > initial_error_count) return; /* Exit after first error */
                        }
                    }
                } else if (strcmp(first_word, "case") != 0 && strcmp(first_word, "default") != 0 && *trimmed_line != '}') {
                    /* It's a statement (but not a label or closing brace) */
                    if (parse_state.brace_depth > 0) {
                        parse_state.statement_seen[parse_state.brace_depth] = 1;
                    }
                }
            }
            free(line_copy);
        }
    }

    /* --- STYLE CHECKS --- */
    if (strlen(original_line) > line_length_limit) {
                    add_error_with_excerpt(filename, line_num, line_length_limit + ARRAY_OFFSET_1, ERROR_STYLE, "Line exceeds maximum length.", original_line);
        if (error_count > initial_error_count) return; /* Exit after first error */
    }
    
    /* Update block state AFTER all checks for the current line are done */
    s = clean_line;
    while(*s) {
        if (*s == '{') {
            if (parse_state.brace_depth < MAX_BLOCK_DEPTH - 1) {
                parse_state.brace_depth++;
                parse_state.statement_seen[parse_state.brace_depth] = 0; /* Reset for new block */
            }
        } else if (*s == '}') {
            if (parse_state.brace_depth > 0) {
                 parse_state.statement_seen[parse_state.brace_depth] = 0; /* Clear old state */
                 parse_state.brace_depth--;
            }
        }
        s++;
    }
    

}

static int process_file(const char *filename) {
    BPTR file_handle;
    static char line_buffer[MAX_LINE_LENGTH]; /* Static to avoid stack allocation in loop */
    int line_num = 0;

    /* Reset state for each new file */
    memset(&parse_state, 0, sizeof(parse_state));

    file_handle = Open(filename, MODE_OLDFILE);
    if (!file_handle) {
        Printf("Error: Cannot open file '%s'\n", filename);
        return 1;
    }

    Printf("Analyzing: %s\n", filename); /* Always show which file is being processed */
    total_files++;

    while (FGets(file_handle, line_buffer, sizeof(line_buffer))) {
        line_num++;
        total_lines++;

        /* Remove newline characters */
        line_buffer[strcspn(line_buffer, "\n\r")] = '\0';

        process_line(line_buffer, line_num, filename);
    }

    Close(file_handle);
    
    if (parse_state.in_multiline_comment) {
        add_error(filename, line_num, 1, ERROR_WARNING, "File ends with an unterminated '/*' comment.");
    }
    
    /* Validate Forbid()/Permit() pairs at end of file */
    validate_forbid_permit_pairs(filename);
    
    return 0;
}

static void print_errors(void) {
    int i;
    const char *type_names[] = {"SYNTAX", "STYLE", "WARNING", "COMPILER", "COMMENT"};

    if (!quiet_mode) Printf("\n--- Detailed Error Report ---\n");
    for (i = 0; i < error_count && i < MAX_ERRORS; i++) {
        Printf("%s:%ld:%ld: [%s] %s\n",
               errors[i].filename,
               errors[i].line_number,
               errors[i].column,
               type_names[errors[i].type],
               errors[i].message);
        
        /* Show line excerpt if available */
        if (errors[i].line_excerpt[0] != '\0') {
            Printf("    | %s\n", errors[i].line_excerpt);
        }
    }
}

static void print_usage(void) {
    /* A little trick to extract the version string from the $VER tag */
    char version_string[32];
    const char *start = strchr(codex_verstag, '(');
    const char *end = strchr(codex_verstag, ')');
    if (start && end && end > start) {
        size_t len = end - (start + VERSION_START_OFFSET);
        if (len < sizeof(version_string)) {
            strncpy(version_string, start + VERSION_START_OFFSET, len);
            version_string[len] = '\0';
        }
    } else {
        strcpy(version_string, "Version unknown"); /* Fallback */
    }

    Printf("Codex - Amiga C Linter & Style Checker (%s)\n", version_string);
    Printf("Usage: Codex FILES/M/A,AMIGA/S,NDK/S,C89/S,C99/S,SASC/S,VBCC/S,DICE/S,MEMSAFE/S,QUIET/S,HELP/S\n\n");

    Printf("  C89/S         Check compliance with ANSI C89 standards (default).\n");
    Printf("  C99/S         Check compliance with C99 standards.\n");
    Printf("  AMIGA/S       Check compliance with Amiga C best practices such as Exec types.\n");
    Printf("  MEMSAFE/S     Check for use of memory-unsafe standard C functions. Implies C89/S.\n\n");
    Printf("  NDK/S         Identify keywords that should be converted to use <clib/compiler-specific.h> macros/defines from the NDK.\n");
    Printf("  SASC/S        Check for SAS/C compatibility. Implies C89/S but allows C++ comments.\n");
    Printf("  VBCC/S        Check for VBCC compatibility. Implies C99/S.\n");
    Printf("  DICE/S        Check for DICE keyword compatibility. Implies C89/S & NDK/S.\n");
    Printf("  QUIET/S       Suppress summary and only output violation lines.\n");
    Printf("  HELP/S        Display this help message.\n\n");

    Printf("--- Examples ---\n");
    Printf("  Codex main.c AMIGA\n");
    Printf("    -> Checks main.c for Amiga standards.\n\n");
    Printf("  Codex #?.c AMIGA\n");
    Printf("    -> Checks all .c files in current directory for Amiga standards.\n\n");
    Printf("  Codex main.c C99 VBCC\n");
    Printf("    -> Checks main.c for C99 and VBCC compatibility.\n\n");
    Printf("  Codex main.c AMIGA C99\n");
    Printf("    -> Checks for Amiga standards using C99 as the base standard.\n\n");
    Printf("  Codex main.c MEMSAFE QUIET\n");
    Printf("    -> Checks main.c for memory safety, printing only the errors.\n");
}

/* Helper to find the first non-whitespace character in a string */
static char* find_first_non_whitespace(char *str) {
    while (*str && isspace((unsigned char)*str)) {
        str++;
    }
    return str;
}

/* Helper to trim leading whitespace from a string - kept for future use */
/* static void trim_leading_whitespace(char *str) {
    char *start;
    
    start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
} */

/* Custom string function implementations removed - using standard library functions */

/* Custom strtok implementation removed - using standard library */

/* Custom strchr implementation removed - using standard library */

/* Custom memmove implementation removed - using standard library */

/* Custom strcmp implementation removed - using standard library */

/* Custom strncmp implementation removed - using standard library */

/* Custom isspace implementation removed - using standard library */

/* Custom isalnum implementation removed - using standard library */

/* Custom islower implementation removed - using standard library */

/* Custom isupper implementation removed - using standard library */

/* Custom isdigit implementation removed - using standard library */

/* Custom memset implementation removed - using standard library */

/* Custom strcpy implementation removed - using standard library */

/* Custom strlen implementation removed - using standard library */

/* Custom strncpy implementation removed - using standard library */

/* Custom strstr implementation removed - using standard library */

/* Custom strncat implementation removed - using standard library */

/* Using standard library malloc/free from stdlib.h */

/* ============================================================================ */
/* STANDARDS VALIDATION FUNCTIONS */
/* ============================================================================ */

/* Check for Amiga coding standards compliance */
static void check_amiga_standards(const char *line, int line_num, const char *filename, const char *original_line) {
    char line_copy[MAX_LINE_LENGTH];
    char *token;
    char *paren_pos;
    
    /* Initialize line_copy for use throughout the function */
    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';
    
    /* Check for standard C types that should use Amiga types */
    /* Use more specific patterns to avoid false positives in strings/comments */
    if ((strstr(line, "char *") && !strstr(line, "\"char *")) || 
        (strstr(line, "char*") && !strstr(line, "\"char*"))) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "Use Amiga types (UBYTE* or STRPTR) instead of char*", original_line);
    }
    
    if ((strstr(line, "long ") && !strstr(line, "\"long ")) || 
        (strstr(line, "long\t") && !strstr(line, "\"long\t"))) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "Use Amiga types (LONG) instead of long", original_line);
    }
    
    if ((strstr(line, "int ") && !strstr(line, "\"int ")) || 
        (strstr(line, "int\t") && !strstr(line, "\"int\t"))) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "Use Amiga types (ULONG) instead of int", original_line);
    }
    
    if ((strstr(line, "short ") && !strstr(line, "\"short ")) || 
        (strstr(line, "short\t") && !strstr(line, "\"short\t"))) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "Use Amiga types (WORD) instead of short", original_line);
    }
    
    if ((strstr(line, "unsigned long") && !strstr(line, "\"unsigned long")) || 
        (strstr(line, "unsigned char") && !strstr(line, "\"unsigned char")) || 
        (strstr(line, "unsigned short") && !strstr(line, "\"unsigned short")) || 
        (strstr(line, "unsigned int") && !strstr(line, "\"unsigned int"))) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_STYLE, 
                 "Use Amiga primitive types (ULONG, UBYTE, UWORD) instead of standard C types", original_line);
    }
    
    /* Check for deprecated Amiga types with warnings */
    if (strstr(line, "USHORT") || strstr(line, "ushort")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "USHORT is deprecated - use UWORD instead", original_line);
    }
    
    if (strstr(line, "SHORT") || strstr(line, "short")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "SHORT is deprecated - use WORD instead", original_line);
    }
    
    if (strstr(line, "COUNT") || strstr(line, "count")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "COUNT is deprecated - use WORD instead", original_line);
    }
    
    if (strstr(line, "UCOUNT") || strstr(line, "ucount")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "UCOUNT is deprecated - use UWORD instead", original_line);
    }
    
    if (strstr(line, "CPTR") || strstr(line, "cptr")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "CPTR is deprecated - use ULONG instead", original_line);
    }
    
    /* Check for other deprecated or problematic types */
    if (strstr(line, "LONGBITS") || strstr(line, "longbits")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "LONGBITS is for bit manipulation - consider if you really need this", original_line);
    }
    
    if (strstr(line, "WORDBITS") || strstr(line, "wordbits")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "WORDBITS is for bit manipulation - consider if you really need this", original_line);
    }
    
    if (strstr(line, "BYTEBITS") || strstr(line, "bytebits")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "BYTEBITS is for bit manipulation - consider if you really need this", original_line);
    }
    
    if (strstr(line, "RPTR") || strstr(line, "rptr")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "RPTR is for relative pointers - consider if you really need this", original_line);
    }
    
    /* Check for proper Amiga types usage */
    if (strstr(line, "float ") || strstr(line, "float\t")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "Use Amiga types (FLOAT) instead of float", original_line);
    }
    
    if (strstr(line, "double ") || strstr(line, "double\t")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "Use Amiga types (DOUBLE) instead of double", original_line);
    }
    
    if (strstr(line, "bool ") || strstr(line, "bool\t")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "Use Amiga types (BOOL) instead of bool", original_line);
    }
    
    /* Check for pointer types that should use Amiga types */
    if (strstr(line, "void *") || strstr(line, "void*")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "Consider using Amiga types (APTR) instead of void* for untyped pointers", original_line);
    }
    
    /* Check for string types that should use Amiga types */
    if (strstr(line, "const char *") || strstr(line, "const char*")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "Use Amiga types (CONST_STRPTR) instead of const char*", original_line);
    }
    
    /* Check for text types that should use Amiga types */
    if (strstr(line, "unsigned char *") || strstr(line, "unsigned char*")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "Use Amiga types (STRPTR) instead of unsigned char* for strings", original_line);
    }
    
    /* Check for PascalCase function definitions (not stdlib functions) */
    token = strtok(line_copy, " \t\n\r");
    
    if (token) {
        /* Check if this looks like a function definition */
        paren_pos = strchr(line, '(');
        
        if (paren_pos) {
            /* This looks like a function definition - get the function name (next token) */
            char *func_name = strtok(NULL, " \t\n\r*(");
            if (func_name) {
                /* Only check PascalCase for non-stdlib and non-Amiga functions */
                if (!is_stdlib_function(func_name) && !is_amiga_function(func_name) && islower((unsigned char)func_name[0])) {
                    add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                                 "Use PascalCase function names", original_line);
                }
            }
        }
    }
    
    /* Check for proper Amiga library usage patterns - double check SAS/C manual before enabling */
    /* if (strstr(line, "exit(") && !strstr(line, "return")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_STYLE, 
                 "Use 'return' instead of 'exit()' to ensure proper RC and Result2 handling", original_line);
    } */
    
    /* Check for assignment of 0 to a pointer, which should be NULL */
    if (strstr(line, "*") && strstr(line, "= 0") && !strstr(line, "== 0")) {
        add_error_with_excerpt(filename, line_num, 1, ERROR_STYLE,
                         "Assigning 0 to a pointer. Use the Amiga constant NULL instead.", original_line);
    }
}

/* Check for NDK compiler-specific.h reserved words */
static void check_ndk_standards(const char *line, int line_num, const char *filename, const char *original_line) {
    char line_copy[MAX_LINE_LENGTH];
    char *token;
    
    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';
    
    token = strtok(line_copy, " \t\n\r");
    
    while (token) {
        if (is_ndk_reserved_word(token)) {
            add_error(filename, line_num, 1, ERROR_COMPILER, 
                     "NDK reserved word found - use universal syntax instead");
        }
        token = strtok(NULL, " \t\n\r");
    }
}

/* Check for C89 compliance */
static void check_c89_standards(const char *line, int line_num, const char *filename, const char *original_line) {
    char *for_pos;
    char *int_pos;
    char *type_pos;
    
    /* Check for C++ comments - but skip if SAS/C mode is active (SAS/C supports them) */
    if (strstr(line, "//") && !validate_sasc_standards) {
        add_error(filename, line_num, 1, ERROR_SYNTAX, 
                 "C++ comments ('//') are not allowed in C89");
    }
    
    /* Check for C99 keywords */
    if (strstr(line, "inline")) {
        add_error(filename, line_num, 1, ERROR_SYNTAX, 
                 "'inline' keyword is not available in C89");
    }
    
    if (strstr(line, "_Bool")) {
        add_error(filename, line_num, 1, ERROR_SYNTAX, 
                 "_Bool type is not available in C89");
    }
    
    if (strstr(line, "restrict")) {
        add_error(filename, line_num, 1, ERROR_SYNTAX, 
                 "'restrict' keyword is not available in C89");
    }
    
    /* Check for variable declarations in for loop initializers */
    if (strstr(line, "for") && strstr(line, "int")) {
        for_pos = strstr(line, "for");
        int_pos = strstr(line, "int");
        if (int_pos > for_pos) {
            add_error(filename, line_num, 1, ERROR_SYNTAX, 
                     "Variable declaration in for loop not allowed in C89");
        }
    }
    
    /* Enhanced for loop detection for other types */
    if (strstr(line, "for") && (strstr(line, "char ") || strstr(line, "long ") || 
                                 strstr(line, "short ") || strstr(line, "float ") || 
                                 strstr(line, "double ") || strstr(line, "unsigned "))) {
        for_pos = strstr(line, "for");
        type_pos = NULL;
        
        if (strstr(line, "char ")) type_pos = strstr(line, "char ");
        else if (strstr(line, "long ")) type_pos = strstr(line, "long ");
        else if (strstr(line, "short ")) type_pos = strstr(line, "short ");
        else if (strstr(line, "float ")) type_pos = strstr(line, "float ");
        else if (strstr(line, "double ")) type_pos = strstr(line, "double ");
        else if (strstr(line, "unsigned ")) type_pos = strstr(line, "unsigned ");
        
        if (type_pos && type_pos > for_pos) {
            add_error(filename, line_num, 1, ERROR_SYNTAX, 
                     "Variable declaration in for loop not allowed in C89");
        }
    }
    
    /* Enhanced C99 feature detection for C89 compliance */
    if (is_c99_designated_init(line)) {
        add_error(filename, line_num, 1, ERROR_SYNTAX, 
                 "C99 designated initializer found - not available in C89");
    }
    
    if (is_c99_compound_literal(line)) {
        add_error(filename, line_num, 1, ERROR_SYNTAX, 
                 "C99 compound literal found - not available in C89");
    }
    
    if (is_c99_variadic_macro(line)) {
        add_error(filename, line_num, 1, ERROR_SYNTAX, 
                 "C99 variadic macro found - not available in C89");
    }
    
    if (is_c99_flexible_array(line)) {
        add_error(filename, line_num, 1, ERROR_SYNTAX, 
                 "C99 flexible array member found - not available in C89");
    }
    
    if (is_c99_stdlib_function(line)) {
        add_error(filename, line_num, 1, ERROR_SYNTAX, 
                 "C99+ standard library function found - not available in C89");
    }
    
    if (is_c99_header_file(line)) {
        add_error(filename, line_num, 1, ERROR_SYNTAX, 
                 "C99+ header file found - not available in C89");
    }
}

/* Check for C99 compliance */
static void check_c99_standards(const char *line, int line_num, const char *filename, const char *original_line) {
    /* In C99 mode, we validate that C99 features are properly used */
    /* Check for C99 keywords - these should be valid in C99 mode */
    if (is_c99_keyword(line)) {
        /* This is fine in C99 mode - just note it for information */
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "C99 keyword detected - ensure your compiler supports C99", original_line);
    }
    
    /* Check for C99 features - these should be valid in C99 mode */
    if (is_c99_feature(line)) {
        /* This is fine in C99 mode - just note it for information */
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "C99 feature detected - ensure your compiler supports C99", original_line);
    }
    
    /* Enhanced C99 feature detection - these should be valid in C99 mode */
    if (is_c99_designated_init(line)) {
        /* This is fine in C99 mode - just note it for information */
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "C99 designated initializer detected - ensure your compiler supports C99", original_line);
    }
    
    if (is_c99_compound_literal(line)) {
        /* This is fine in C99 mode - just note it for information */
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "C99 compound literal detected - ensure your compiler supports C99", original_line);
    }
    
    if (is_c99_variadic_macro(line)) {
        /* This is fine in C99 mode - just note it for information */
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "C99 variadic macro detected - ensure your compiler supports C99", original_line);
    }
    
    if (is_c99_flexible_array(line)) {
        /* This is fine in C99 mode - just note it for information */
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "C99 flexible array member detected - ensure your compiler supports C99", original_line);
    }
    
    if (is_c99_stdlib_function(line)) {
        /* This is fine in C99 mode - just note it for information */
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "C99+ standard library function detected - ensure your compiler supports C99", original_line);
    }
    
    if (is_c99_header_file(line)) {
        /* This is fine in C99 mode - just note it for information */
        add_error_with_excerpt(filename, line_num, 1, ERROR_WARNING, 
                 "C99+ header file detected - ensure your compiler supports C99", original_line);
    }
    
    /* Check for C89 header files when in C99 mode */
    if (is_c89_header_file(line)) {
        /* This is fine - C89 headers are allowed in C99 mode */
    }
}

/* Check for SAS/C compliance */
static void check_sasc_standards(const char *line, int line_num, const char *filename, const char *original_line) {
    char line_copy[MAX_LINE_LENGTH];
    char *token;
    
    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';
    
    token = strtok(line_copy, " \t\n\r*();,");
    
    while (token) {
        if (is_sasc_keyword(token)) {
            char replacement[REPLACEMENT_BUFFER_SIZE];
            char message[256];
            if (find_universal_replacement(token, replacement, sizeof(replacement)) && strcmp(replacement, "(none)") != 0) {
                strncpy(message, "Keyword '", sizeof(message) - 1);
                strncat(message, token, sizeof(message) - strlen(message) - 1);
                strncat(message, "' is incompatible with SAS/C. Use universal syntax '", sizeof(message) - strlen(message) - 1);
                strncat(message, replacement, sizeof(message) - strlen(message) - 1);
                strncat(message, "' instead.", sizeof(message) - strlen(message) - 1);
            } else {
                strncpy(message, "Keyword '", sizeof(message) - 1);
                strncat(message, token, sizeof(message) - strlen(message) - 1);
                strncat(message, "' is incompatible with SAS/C and has no direct universal equivalent.", sizeof(message) - strlen(message) - 1);
            }
            add_error(filename, line_num, 1, ERROR_COMPILER, message);
            return;
        }
        token = strtok(NULL, " \t\n\r*();,");
    }
}

/* Check for VBCC compliance */
static void check_vbcc_standards(const char *line, int line_num, const char *filename, const char *original_line) {
    char line_copy[MAX_LINE_LENGTH];
    char *token;

    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';

    token = strtok(line_copy, " \t\n\r*();,");

    while (token) {
        if (is_vbcc_keyword(token)) {
            char replacement[REPLACEMENT_BUFFER_SIZE];
            char message[256];
            if (find_universal_replacement(token, replacement, sizeof(replacement)) && strcmp(replacement, "(none)") != 0) {
                strncpy(message, "Keyword '", sizeof(message) - 1);
                strncat(message, token, sizeof(message) - strlen(message) - 1);
                strncat(message, "' is incompatible with VBCC. Use universal syntax '", sizeof(message) - strlen(message) - 1);
                strncat(message, replacement, sizeof(message) - strlen(message) - 1);
                strncat(message, "' instead.", sizeof(message) - strlen(message) - 1);
            } else {
                strncpy(message, "Keyword '", sizeof(message) - 1);
                strncat(message, token, sizeof(message) - strlen(message) - 1);
                strncat(message, "' is incompatible with VBCC and has no direct universal equivalent.", sizeof(message) - strlen(message) - 1);
            }
            add_error(filename, line_num, 1, ERROR_COMPILER, message);
            return;
        }
        token = strtok(NULL, " \t\n\r*();,");
    }
}

/* Helper function to check if a word is an NDK reserved word */
static int is_ndk_reserved_word(const char *word) {
    int i;
    int num_words = sizeof(ndk_reserved_words) / sizeof(ndk_reserved_words[0]);
    
    for (i = 0; i < num_words; i++) {
        if (strcmp(word, ndk_reserved_words[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Helper function to check if a word is a C99 keyword */
static int is_c99_keyword(const char *word) {
    int i;
    int num_keywords = sizeof(c99_keywords) / sizeof(c99_keywords[0]);
    
    for (i = 0; i < num_keywords; i++) {
        if (strstr(word, c99_keywords[i])) {
            return 1;
        }
    }
    return 0;
}

/* Helper function to check if a line contains C99 features */
static int is_c99_feature(const char *line) {
    int i;
    int num_features = sizeof(c99_features) / sizeof(c99_features[0]);
    
    for (i = 0; i < num_features; i++) {
        if (strstr(line, c99_features[i])) {
            return 1;
        }
    }
    return 0;
}

/* Helper function to check if a line contains C99 designated initializers */
static int is_c99_designated_init(const char *line) {
    int i;
    int num_patterns = sizeof(c99_designated_init_patterns) / sizeof(c99_designated_init_patterns[0]);
    
    for (i = 0; i < num_patterns; i++) {
        if (strstr(line, c99_designated_init_patterns[i])) {
            return 1;
        }
    }
    return 0;
}

/* Helper function to check if a line contains C99 compound literals */
static int is_c99_compound_literal(const char *line) {
    int i;
    int num_patterns = sizeof(c99_compound_literal_patterns) / sizeof(c99_compound_literal_patterns[0]);
    
    for (i = 0; i < num_patterns; i++) {
        if (strstr(line, c99_compound_literal_patterns[i])) {
            return 1;
        }
    }
    return 0;
}

/* Helper function to check if a line contains C99 variadic macros */
static int is_c99_variadic_macro(const char *line) {
    int i;
    int num_patterns = sizeof(c99_variadic_macro_patterns) / sizeof(c99_variadic_macro_patterns[0]);
    
    for (i = 0; i < num_patterns; i++) {
        if (strstr(line, c99_variadic_macro_patterns[i])) {
            return 1;
        }
    }
    return 0;
}

/* Helper function to check if a line contains C99 flexible array members */
static int is_c99_flexible_array(const char *line) {
    int i;
    int num_patterns = sizeof(c99_flexible_array_patterns) / sizeof(c99_flexible_array_patterns[0]);
    
    for (i = 0; i < num_patterns; i++) {
        if (strstr(line, c99_flexible_array_patterns[i])) {
            return 1;
        }
    }
    return 0;
}

/* Helper function to check if a line contains C99+ standard library functions */
static int is_c99_stdlib_function(const char *line) {
    int i;
    int num_functions = sizeof(c99_stdlib_functions) / sizeof(c99_stdlib_functions[0]);
    
    for (i = 0; i < num_functions; i++) {
        if (strstr(line, c99_stdlib_functions[i])) {
            return 1;
        }
    }
    return 0;
}

/* Helper function to check if a line contains C99+ header files */
static int is_c99_header_file(const char *line) {
    int i;
    int num_headers = sizeof(c99_header_files) / sizeof(c99_header_files[0]);
    
    for (i = 0; i < num_headers; i++) {
        if (strstr(line, c99_header_files[i])) {
            return 1;
        }
    }
    return 0;
}

/* Helper function to check if a word is a SAS/C keyword */
static int is_sasc_keyword(const char *word) {
    int i;
    int num_keywords = sizeof(sasc_keywords) / sizeof(sasc_keywords[0]);
    
    for (i = 0; i < num_keywords; i++) {
        if (strcmp(word, sasc_keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Helper function to check if a word is a VBCC keyword */
static int is_vbcc_keyword(const char *word) {
    int i;
    int num_keywords = sizeof(vbcc_keywords) / sizeof(vbcc_keywords[0]);
    
    for (i = 0; i < num_keywords; i++) {
        if (strcmp(word, vbcc_keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Helper function to check if a line contains C89 header files */
static int is_c89_header_file(const char *line) {
    int i;
    int num_headers = sizeof(c89_header_files) / sizeof(c89_header_files[0]);
    
    for (i = 0; i < num_headers; i++) {
        if (strstr(line, c89_header_files[i])) {
            return 1;
        }
    }
    return 0;
}

/* Check for DICE compiler compatibility */
static void check_dice_standards(const char *line, int line_num, const char *filename, const char *original_line) {
    /* DICE mode currently implements C89 + compiler keywords */
    /* This will be expanded for full DICE compiler compatibility */
    char line_copy[MAX_LINE_LENGTH];
    char *token;

    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';

    token = strtok(line_copy, " \t\n\r*();,");

    while (token) {
        if (is_ndk_reserved_word(token)) {
            char replacement[REPLACEMENT_BUFFER_SIZE];
            char message[256];
            if (find_universal_replacement(token, replacement, sizeof(replacement)) && strcmp(replacement, "(none)") != 0) {
                strncpy(message, "Keyword '", sizeof(message) - 1);
                strncat(message, token, sizeof(message) - strlen(message) - 1);
                strncat(message, "' is DICE-incompatible. Use universal syntax '", sizeof(message) - strlen(message) - 1);
                strncat(message, replacement, sizeof(message) - strlen(message) - 1);
                strncat(message, "' instead.", sizeof(message) - strlen(message) - 1);
            } else {
                strncpy(message, "Keyword '", sizeof(message) - 1);
                strncat(message, token, sizeof(message) - strlen(message) - 1);
                strncat(message, "' is DICE-incompatible and has no direct universal equivalent.", sizeof(message) - strlen(message) - 1);
            }
            add_error(filename, line_num, 1, ERROR_COMPILER, message);
            return;
        }
        token = strtok(NULL, " \t\n\r*();,");
    }
}

/* Check for memory safety issues */
static void check_memsafe_standards(const char *line, int line_num, const char *filename, const char *original_line) {
    char line_copy[MAX_LINE_LENGTH];
    char *token;
    char replacement[256];
    
    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';
    
    token = strtok(line_copy, " \t\n\r*();,");
    
    while (token) {
        if (is_memsafe_unsafe_function(token)) {
            if (find_memsafe_replacement(token, replacement, sizeof(replacement))) {
                char message[LARGE_MESSAGE_BUFFER_SIZE];

                /* --- NEW: Add qualified guidance for specific functions --- */
                if (strcmp(token, "realpath") == 0) {
                    strncpy(message, "Unsafe use of 'realpath' suspected. Ensure the second argument is a valid buffer, not NULL.", sizeof(message) - 1);
                } else if (strcmp(token, "scanf") == 0 || strcmp(token, "sscanf") == 0) {
                    strncpy(message, "Unsafe use of '", sizeof(message) - 1);
                    strncat(message, token, sizeof(message) - strlen(message) - 1);
                    strncat(message, "' suspected. Ensure format string uses width specifiers (e.g., '%10s') and check the return value.", sizeof(message) - strlen(message) - 1);
                } else {
                    /* --- Fallback to the original generic message --- */
                    strncpy(message, "Memory-unsafe function '", sizeof(message) - 1);
                    strncat(message, token, sizeof(message) - strlen(message) - 1);
                    strncat(message, "' found - consider using '", sizeof(message) - strlen(message) - 1);
                    strncat(message, replacement, sizeof(message) - strlen(message) - 1);
                    strncat(message, "' instead", sizeof(message) - strlen(message) - 1);
                }
                
                message[sizeof(message) - 1] = '\0'; /* Ensure null termination */
                add_error(filename, line_num, 1, ERROR_WARNING, message);
                
                return;
            }
        }
        token = strtok(NULL, " \t\n\r*();,");
    }
}

/* Helper function to check if a function is memory-unsafe */
static int is_memsafe_unsafe_function(const char *word) {
    int i;
    int num_functions = sizeof(memsafe_unsafe_functions) / sizeof(memsafe_unsafe_functions[0]);
    
    for (i = 0; i < num_functions; i++) {
        if (strcmp(word, memsafe_unsafe_functions[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Helper function to check if a function is a standard library function */
static int is_stdlib_function(const char *word) {
    int i;
    int num_functions = sizeof(stdlib_functions) / sizeof(stdlib_functions[0]);
    
    for (i = 0; i < num_functions; i++) {
        if (strcmp(word, stdlib_functions[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Helper function to check if a function is a common Amiga library function */
static int is_amiga_function(const char *word) {
    int i;
    int num_functions = sizeof(amiga_functions) / sizeof(amiga_functions[0]);
    
    for (i = 0; i < num_functions; i++) {
        if (strcmp(word, amiga_functions[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Helper function to find memory-safe replacement for unsafe function */
static int find_memsafe_replacement(const char *function, char *replacement, size_t max_len) {
    int i;
    int num_functions = sizeof(memsafe_unsafe_functions) / sizeof(memsafe_unsafe_functions[0]);
    
    for (i = 0; i < num_functions; i++) {
        if (strcmp(function, memsafe_unsafe_functions[i]) == 0) {
            strncpy(replacement, memsafe_safe_replacements[i], max_len - 1);
            replacement[max_len - 1] = '\0';
            return 1;
        }
    }
    return 0;
}

/* Helper to find the universal replacement for a compiler-specific keyword */
static int find_universal_replacement(const char *keyword, char *replacement, size_t max_len) {
    int i;
    int num_keywords = sizeof(non_universal_keywords) / sizeof(non_universal_keywords[0]);
    for (i = 0; i < num_keywords; i++) {
        if (strcmp(keyword, non_universal_keywords[i]) == 0) {
            strncpy(replacement, universal_replacements[i], max_len - 1);
            replacement[max_len - 1] = '\0';
            return 1;
        }
    }
    return 0;
}

/* Check for Forbid()/Permit() pairs on each line */
static void check_forbid_permit_pairs(const char *line, int line_num, const char *filename, const char *original_line) {
    const char *forbid_pos;
    const char *permit_pos;
    int line_distance;
    
    /* Check for Forbid() call - look for "Forbid(" or "Forbid (" */
    forbid_pos = strstr(line, "Forbid(");
    if (!forbid_pos) {
        forbid_pos = strstr(line, "Forbid (");
    }
    
    /* Check for Permit() call - look for "Permit(" or "Permit (" */
    permit_pos = strstr(line, "Permit(");
    if (!permit_pos) {
        permit_pos = strstr(line, "Permit (");
    }
    
    /* Handle case where both Forbid() and Permit() are on the same line */
    if (forbid_pos && permit_pos) {
        /* Check which comes first */
        if (forbid_pos < permit_pos) {
            /* Forbid() comes first - process in order */
            parse_state.forbid_count++;
            if (parse_state.forbid_active) {
                /* Multiple Forbid() calls without Permit() in between */
                add_error_with_excerpt(filename, line_num, (forbid_pos - line) + ARRAY_OFFSET_1, ERROR_WARNING,
                                     "Forbid() called without matching Permit() from previous Forbid()", original_line);
            } else {
                parse_state.forbid_active = 1;
                parse_state.forbid_line = line_num;
                /* Warn that Forbid() is being used */
                add_error_with_excerpt(filename, line_num, (forbid_pos - line) + ARRAY_OFFSET_1, ERROR_WARNING,
                                     "Forbid() usage detected", original_line);
            }
            /* Now process Permit() */
            parse_state.permit_count++;
            if (parse_state.forbid_active) {
                /* Check if too many lines between Forbid() and Permit() */
                line_distance = line_num - parse_state.forbid_line;
                if (line_distance > 5) {
                    add_error_with_excerpt(filename, line_num, (permit_pos - line) + ARRAY_OFFSET_1, ERROR_WARNING,
                                         "Too many lines (>5) between Forbid() and Permit()", original_line);
                }
                parse_state.permit_line = line_num;
                parse_state.forbid_active = 0; /* Reset for next pair */
            }
        } else {
            /* Permit() comes first - this is an error */
            parse_state.permit_count++;
            add_error_with_excerpt(filename, line_num, (permit_pos - line) + ARRAY_OFFSET_1, ERROR_WARNING,
                                 "Permit() called without matching Forbid()", original_line);
            /* Process Forbid() after */
            parse_state.forbid_count++;
            parse_state.forbid_active = 1;
            parse_state.forbid_line = line_num;
            add_error_with_excerpt(filename, line_num, (forbid_pos - line) + ARRAY_OFFSET_1, ERROR_WARNING,
                                 "Forbid() usage detected", original_line);
        }
        return; /* Both processed, exit early */
    }
    
    /* Warn if Forbid() is used */
    if (forbid_pos) {
        parse_state.forbid_count++;
        if (parse_state.forbid_active) {
            /* Multiple Forbid() calls without Permit() in between */
            add_error_with_excerpt(filename, line_num, (forbid_pos - line) + ARRAY_OFFSET_1, ERROR_WARNING,
                                 "Forbid() called without matching Permit() from previous Forbid()", original_line);
        } else {
            parse_state.forbid_active = 1;
            parse_state.forbid_line = line_num;
            /* Warn that Forbid() is being used */
            add_error_with_excerpt(filename, line_num, (forbid_pos - line) + ARRAY_OFFSET_1, ERROR_WARNING,
                                 "Forbid() usage detected", original_line);
        }
    }
    
    /* Warn if Permit() is used */
    if (permit_pos) {
        parse_state.permit_count++;
        if (!parse_state.forbid_active) {
            /* Permit() called without matching Forbid() */
            add_error_with_excerpt(filename, line_num, (permit_pos - line) + ARRAY_OFFSET_1, ERROR_WARNING,
                                 "Permit() called without matching Forbid()", original_line);
        } else {
            /* Check if too many lines between Forbid() and Permit() */
            line_distance = line_num - parse_state.forbid_line;
            if (line_distance > 5) {
                add_error_with_excerpt(filename, line_num, (permit_pos - line) + ARRAY_OFFSET_1, ERROR_WARNING,
                                     "Too many lines (>5) between Forbid() and Permit()", original_line);
            }
            parse_state.permit_line = line_num;
            parse_state.forbid_active = 0; /* Reset for next pair */
        }
    }
}

/* Validate Forbid()/Permit() pairs at end of file */
static void validate_forbid_permit_pairs(const char *filename) {
    /* Warn if Forbid()/Permit() are used at all */
    if (parse_state.forbid_count > 0 || parse_state.permit_count > 0) {
        if (parse_state.forbid_count > 0 && parse_state.permit_count == 0) {
            add_error(filename, parse_state.forbid_line, 1, ERROR_WARNING,
                     "Forbid() used without matching Permit()");
        } else if (parse_state.forbid_count == 0 && parse_state.permit_count > 0) {
            /* This case is already handled in check_forbid_permit_pairs */
        } else if (parse_state.forbid_count != parse_state.permit_count) {
            add_error(filename, 1, 1, ERROR_WARNING,
                     "Mismatched Forbid()/Permit() pairs: count mismatch");
        }
        
        /* Warn if file ends with active Forbid() */
        if (parse_state.forbid_active) {
            add_error(filename, parse_state.forbid_line, 1, ERROR_WARNING,
                     "File ends with active Forbid() without matching Permit()");
        }
    }
}

/* Check for magic numbers - hardcoded numerical constants that should be named constants */
static void check_for_magic_numbers(const char *line, int line_num, const char *filename, const char *original_line) {
    const char *p = line;
    int in_string = 0;

    while (*p) {
        if (*p == '"') in_string = !in_string;

        /* Check for a digit that is not inside a string and is not part of a word */
        if (!in_string && isdigit((unsigned char)*p)) {
            /* A simple check: is the digit preceded by an operator or parenthesis? */
            if (p > line && (strchr("+-*/%=(<>,", *(p - PREVIOUS_CHAR_OFFSET)))) {
                /* Avoid flagging array initializers like { 1, 2, 3 } */
                if (!strchr("{,", *(p-PREVIOUS_CHAR_OFFSET))) {
                     add_error_with_excerpt(filename, line_num, (p - line) + ARRAY_OFFSET_1, ERROR_STYLE,
                                     "Magic number found. Consider using a named constant.", original_line);
                     return; /* Only flag one per line */
                }
            }
        }
        p++;
    }
}
# Codex - Multi-Standard Code Guardian

A comprehensive code linting tool for AmigaOS development that enforces multiple coding standards and ensures cross-compiler compatibility.

## Overview

Codex is a lightweight code guardian that analyzes C source code for:
- Basic syntax errors and style violations
- C89/C99 compliance issues
- Compiler-specific compatibility problems
- Amiga coding standards violations
- Memory safety concerns

## Modes and Functionality

### Default Mode (Always Active)
**Basic C Linting** - Always enabled with no arguments, performs fundamental syntax checking:
- Missing semicolons
- Unmatched brackets and braces
- Basic syntax errors
- Style violations (line length, trailing whitespace, etc.)

### C89 Mode (`C89/S`)
When enabled, checks for common C89 AND C99 violations:
- Variable declarations not at start of blocks
- Missing return statements in non-void functions
- C++ style comments (`//`)
- C99 keywords (`inline`, `restrict`, `_Bool`, etc.)
- C99 features (designated initializers, compound literals, variadic macros)
- C99+ standard library functions (`snprintf`, `strdup`, etc.)
- C99+ header files (`<stdint.h>`, `<stdbool.h>`, etc.)
- **Header Validation**: Ensures included headers are C89 stdlib headers, not later versions

### C99 Mode (`C99/S`)
When enabled, checks for C99 violations only:
- C99-specific keywords and features
- **Header Validation**: Ensures included headers are C99 versions, not C89 versions
- Note: Not necessary to specify when C89 mode is on (C89 mode covers C99 violations)

### SAS/C Mode (`SASC/S`)
Flags SAS/C incompatibilities beyond C89 ones:
- **Compiler Keywords**: Flags any DICE, GCC, or VBCC specific keywords from `compiler-specific.h`
- **Implies C89 Mode**: Automatically enables C89 compliance checking
- Ensures code will compile with SAS/C compiler

### VBCC Mode (`VBCC/S`)
Flags VBCC incompatibilities beyond C89 ones:
- **Compiler Keywords**: Flags any DICE, GCC, or SAS/C specific keywords from `compiler-specific.h`
- **Implies C99 Mode**: Automatically enables C99 compliance checking (not C89)
- Ensures code will compile with VBCC compiler

### DICE Mode (`DICE/S`)
**Future Mode** - Currently implements:
- C89 compliance checking
- Compiler-specific keyword validation from `compiler-specific.h`
- Will be expanded for full DICE compiler compatibility

### AMIGA Mode (`AMIGA/S`)
Checks for compliance with Amiga coding standards:
- **Primitive Types**: Flags use of generic C types instead of Amiga types
- **Required Types**: `UBYTE`, `ULONG`, `UWORD`, `LONG`, `BYTE`, `WORD` (from `exec/types.h`)
- **Flagged Types**: `char`, `int`, `long`, `unsigned char`, `unsigned int`, `unsigned long`
- **Implies NDK Mode**: Automatically enables NDK compiler-specific.h validation
- **Function Naming**: Enforces PascalCase for Amiga functions
- **Brace Style**: Enforces Allman style (braces on separate lines)

### NDK Mode (`NDK/S`)
Enforces universal keyword syntax from `compiler-specific.h`:
- **Compiler Keywords**: Flags compiler-specific keywords that should use universal syntax
- **Universal Replacements**: Suggests `__SAVE_DS__` instead of `__saveds`, `__ASM__` instead of `__asm`, etc.
- **Cross-Compiler Compatibility**: Ensures code works with multiple Amiga compilers

### MEMSAFE Mode (`MEMSAFE/S`)
**New Mode** - Flags memory-unsafe C standard library functions:
- **String Functions**: Flags unsafe functions like `strcpy`, `strcat`, `sprintf`
- **Safe Replacements**: Suggests memory-safe alternatives like `strncpy`, `strncat`, `snprintf`
- **Buffer Overflow Prevention**: Identifies potential security vulnerabilities
- **Memory Management**: Flags unsafe memory operations

## Command Line Usage

```bash
# Basic usage (default mode only)
codex source.c

# Enable specific modes
codex source.c C89 AMIGA
codex source.c SASC NDK
codex source.c VBCC MEMSAFE

# Disable specific checks
codex source.c AMIGA NO-BRACES-STYLE
codex source.c C89 NO-MACRO-CAPS

# Multiple files with wildcards
codex Sources/#?.c AMIGA NDK C89
```

## Mode Dependencies

- **SASC Mode** → Implies **C89 Mode**
- **VBCC Mode** → Implies **C99 Mode** (not C89)
- **AMIGA Mode** → Implies **NDK Mode**
- **Default Mode** → Always active

## Examples

### C89 Violations
```c
void function() {
    int x = 10;
    int y;  // ← Flagged: Variable declaration not at start of block
    y = x + 5;
}

// ← Flagged: C++ style comment not allowed in C89
```

### $CODEX: Test Comments
```c
/* Test files can include expected output using $CODEX: comments */
int y; /* $CODEX: This should trigger a warning: Variable declaration not at start of block */
strcpy(dest, src); /* $CODEX: This should trigger a warning: Use strncpy for safety */
```

### Compiler Compatibility
```c
__saveds void handler() {  // ← Flagged: Use __SAVE_DS__ instead
    __asm("nop");          // ← Flagged: Use __ASM__ instead
}
```

### Amiga Standards
```c
char buffer[256];          // ← Flagged: Use UBYTE instead
long value;                // ← Flagged: Use LONG instead
printf("Hello");           // ← Flagged: Use Printf (PascalCase)
```

### Memory Safety
```c
strcpy(dest, src);         // ← Flagged: Use strncpy for safety
sprintf(buffer, "%s", str); // ← Flagged: Use snprintf for safety
```

## Configuration Options

- `NO-MACRO-CAPS/S` - Disable macro naming enforcement (ALL_CAPS)
- `NO-STDLIB-LOWERCASE/S` - Disable stdlib function naming enforcement
- `NO-AMIGA-PASCALCASE/S` - Disable Amiga function naming enforcement
- `NO-BRACES-STYLE/S` - Disable brace style enforcement (Allman style)
- `NO-INDENTATION/S` - Disable indentation style enforcement
- `NO-COMPILER-COMPAT/S` - Disable compiler-specific keyword compatibility check

## Return Codes

- `0` - No issues found
- `5` - Warnings found (style issues, non-critical violations)
- `20` - Errors found (syntax errors, critical violations)

## Building

```bash
# Compile with SAS/C
sc -o Codex codex.c

# Or use the provided SMakefile
smake
```

## Testing

Comprehensive test suite is available in the `unittests/` folder:

```bash
# Quick test run (automatically builds if needed)
./run_tests.sh

# Manual test run
chmod +x unittests/test_codex.sh
./unittests/test_codex.sh

# Run individual tests
./Codex unittests/test_c89_violations.c C89
./Codex unittests/test_memsafe.c MEMSAFE
./Codex unittests/test_headers.c C99
./Codex unittests/demo_codex_comments.c C89 AMIGA
```

See `unittests/TEST_README.md` for detailed testing information.

## License

[Add your license information here]

## Contributing

[Add contribution guidelines here]

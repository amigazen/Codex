# Codex Test Suite

This directory contains comprehensive test files and scripts to verify the functionality of the Codex Multi-Standard Code Guardian.

## Test Files

### 1. `test_example.c`
- **Purpose**: General test file with mixed compiler-specific keywords
- **Contains**: Both SAS/C and VBCC specific keywords, universal syntax examples
- **Use Case**: Testing multiple validation modes simultaneously

### 2. `test_sasc_specific.c`
- **Purpose**: Test SAS/C specific keywords
- **Contains**: `__amigainterrupt`, `__stkargs`, `__attribute__`, `__builtin_`, `__volatile__`, `__const__`, `__restrict__`
- **Expected Behavior**: Should trigger warnings when checking VBCC compatibility, NOT when checking SAS/C compatibility

### 3. `test_vbcc_specific.c`
- **Purpose**: Test VBCC specific keywords
- **Contains**: `__saveds`, `__save_ds`, `__stkargs`, `__attribute__`, `__builtin_`, `__volatile__`, `__const__`, `__restrict__`
- **Expected Behavior**: Should trigger warnings when checking SAS/C compatibility, NOT when checking VBCC compatibility

### 4. `test_c89_violations.c`
- **Purpose**: Test C89 compliance violations
- **Contains**: Variable declarations not at start of blocks, missing return statements, C99+ headers and functions
- **Expected Behavior**: Should trigger warnings when C89 mode is enabled

### 5. `test_amiga_standards.c`
- **Purpose**: Test Amiga coding standards violations
- **Contains**: Lowercase function names, incorrect types, wrong brace style, pointer assignments to 0
- **Expected Behavior**: Should trigger warnings when AMIGA mode is enabled

### 6. `test_memsafe.c`
- **Purpose**: Test memory safety validation
- **Contains**: Memory-unsafe functions like `strcpy`, `sprintf`, `gets`, `tmpnam`, `atoi`, `atol`, `atof`, `getenv`
- **Expected Behavior**: Should trigger warnings when MEMSAFE mode is enabled, suggesting safe alternatives

### 7. `test_headers.c`
- **Purpose**: Test header file validation
- **Contains**: C89 headers, C99+ headers, C99+ types and functions
- **Expected Behavior**: C89 mode should flag C99+ headers, C99 mode should allow both

### 8. `test_compiler_keywords.c`
- **Purpose**: Test compiler keyword compatibility
- **Contains**: SAS/C, VBCC, DICE, GCC specific keywords, universal syntax
- **Expected Behavior**: Should flag appropriate keywords based on compiler mode

## Test Script

### `run_unittests`
A comprehensive Amiga shell script that tests all validation modes:

1. **Basic Usage**: Tests help and error handling
2. **C89 Validation**: Tests C89 compliance checking
3. **SAS/C Compatibility**: Tests detection of VBCC/DICE/GCC keywords for SAS/C compatibility
4. **VBCC Compatibility**: Tests detection of SAS/C/DICE/GCC keywords for VBCC compatibility
5. **Amiga Standards**: Tests Amiga coding standards enforcement
6. **Multiple Validation Modes**: Tests combination of multiple validation modes
7. **NDK Validation**: Tests universal keyword syntax enforcement
8. **DICE Mode**: Tests DICE compiler compatibility
9. **MEMSAFE Mode**: Tests memory safety validation
10. **All Modes**: Tests all validation modes simultaneously
11. **Multiple File Matching**: Tests file pattern matching

## Expected Behavior

### Compiler Compatibility Checks
- **SAS/C mode**: Should flag VBCC/DICE/GCC-specific keywords as compatibility issues
- **VBCC mode**: Should flag SAS/C/DICE/GCC-specific keywords as compatibility issues
- **Universal syntax** (e.g., `__SAVE_DS__`): Should NOT trigger warnings in any mode

### C89 Compliance
- Should flag variable declarations not at start of blocks
- Should flag missing return statements in non-void functions
- Should flag C99+ headers
- Should flag C99+ standard library functions
- Should flag C99 features (designated initializers, compound literals, variadic macros, flexible arrays)
- Should flag other C89 violations

### C99 Compliance
- Should flag C99-specific keywords and features
- Should allow both C89 and C99 headers
- Should flag C99 violations appropriately
- Should provide warnings about C99 features requiring compiler support

### NDK Mode (Universal Keywords)
- Should flag compiler-specific keywords (e.g., `__saveds`, `__asm`)
- Should suggest universal replacements (e.g., `__SAVE_DS__`, `__ASM__`)
- Should help ensure cross-compiler compatibility via compiler-specific.h

### Amiga Standards
- Should flag lowercase function names (should be PascalCase)
- Should flag incorrect type usage (should use Amiga types)
- Should flag incorrect brace style (should use Allman style)
- Should flag assignment of 0 to pointers (should use NULL)

### DICE Mode
- Should flag compiler-specific keywords using NDK validation
- Should imply C89 mode for now
- Will be expanded for full DICE compiler compatibility

### MEMSAFE Mode
- Should flag memory-unsafe functions like `strcpy`, `sprintf`, `gets`, `atoi`, `atol`, `atof`, `getenv`
- Should suggest safe alternatives like `strncpy`, `snprintf`, `fgets`, `strtol`, `strtod`, `getenv_s`
- Should help prevent buffer overflow vulnerabilities
- Should provide function-specific guidance for `realpath`, `scanf`, and `sscanf`

### Magic Number Detection
- Should flag hardcoded numerical constants that should be named constants
- Should avoid flagging array initializers like `{ 1, 2, 3 }`
- Should only flag one magic number per line

### Mode Dependencies
- **SASC Mode** → Implies **C89 Mode** (not C99)
- **VBCC Mode** → Implies **C99 Mode** (not C89)
- **AMIGA Mode** → Implies **NDK Mode**
- **DICE Mode** → Implies **C89 Mode**
- **MEMSAFE Mode** → Implies **C89 Mode**

### Header Validation
- **C89 Mode**: Should flag C99+ headers, allow C89 headers
- **C99 Mode**: Should allow both C89 and C99 headers
- Helps ensure proper header usage for target C standard

### Multiple Mode Combination
- Multiple validation modes should work together
- Total issue count should reflect all active validations
- Disable options should override specific validations

## Running the Tests

```bash
# Make the test script executable
chmod +x unittests/run_unittests

# Run the complete test suite
./unittests/run_unittests

# Or run individual tests
./Codex unittests/test_sasc_specific.c VBCC
./Codex unittests/test_vbcc_specific.c SASC
./Codex unittests/test_c89_violations.c C89
./Codex unittests/test_amiga_standards.c AMIGA
./Codex unittests/test_example.c NDK
./Codex unittests/test_memsafe.c MEMSAFE
./Codex unittests/test_headers.c C89
./Codex unittests/test_compiler_keywords.c SASC
```

## Key Concepts

### $CODEX: Comments
**Special comment syntax for test validation:**
- Lines containing `$CODEX:` comments are processed specially
- Text following `$CODEX:` (up to 256 characters) is output as a comment message
- Allows test files to include expected output for easier validation
- Format: `/* $CODEX: Expected comment message here */`
- Example: `int y; /* $CODEX: This should trigger a warning: Variable declaration not at start of block */`

### Compiler Compatibility
**Compiler compatibility checks work in the opposite direction of what might seem intuitive:**
- When checking SAS/C compatibility, Codex flags keywords that are NOT compatible with SAS/C
- When checking VBCC compatibility, Codex flags keywords that are NOT compatible with VBCC
- This helps developers identify code that won't compile with their target compiler

### Memory Safety
**MEMSAFE mode identifies potential security vulnerabilities:**
- Flags functions known to cause buffer overflows
- Suggests safer alternatives with buffer size parameters
- Provides function-specific guidance for common unsafe patterns
- Helps prevent common security issues in C code

### Header Validation
**Header validation ensures proper C standard compliance:**
- C89 mode enforces use of C89 standard library headers
- C99 mode allows both C89 and C99 headers (backward compatibility)
- Prevents accidental use of newer features in older C standard code

### Magic Number Detection
**Magic number detection encourages better code practices:**
- Flags hardcoded numerical constants that should be named constants
- Avoids false positives in array initializers and other legitimate contexts
- Helps improve code readability and maintainability

### Enhanced C99 Feature Detection
**Comprehensive C99 feature detection:**
- Designated initializers (e.g., `= { .x = 1, .y = 2 }`)
- Compound literals (e.g., `(int[]){ 1, 2, 3 }`)
- Variadic macros (e.g., `__VA_ARGS__`)
- Flexible array members (e.g., `char data[];`)
- C99+ standard library functions and headers

The goal is to ensure code can be compiled with the specified compiler and C standard, not to validate that it uses that compiler's specific features.

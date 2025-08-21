# Codex Test Suite

This directory contains comprehensive test files and scripts to verify the functionality of the Codex style checker.

## Test Files

### 1. `test_example.c`
- **Purpose**: General test file with mixed compiler-specific keywords
- **Contains**: Both SAS/C and VBCC specific keywords
- **Use Case**: Testing multiple validation modes simultaneously

### 2. `test_sasc_specific.c`
- **Purpose**: Test SAS/C specific keywords
- **Contains**: `__saveds`, `__save_ds`, `__stdargs`, `__interrupt`, `__amigainterrupt`, `__asm`, `__reg`, `__far`, `__chip`, `__fast`
- **Expected Behavior**: Should trigger warnings when checking VBCC compatibility, NOT when checking SAS/C compatibility

### 3. `test_vbcc_specific.c`
- **Purpose**: Test VBCC specific keywords
- **Contains**: `__asm`, `__interrupt`, `__amigainterrupt`, `__stdargs`, `__saveds`, `__reg`, `__chip`, `__fast`
- **Expected Behavior**: Should trigger warnings when checking SAS/C compatibility, NOT when checking VBCC compatibility

### 4. `test_c89_violations.c`
- **Purpose**: Test C89 compliance violations
- **Contains**: Variable declarations not at start of blocks, missing return statements
- **Expected Behavior**: Should trigger warnings when C89 mode is enabled

### 5. `test_amiga_standards.c`
- **Purpose**: Test Amiga coding standards violations
- **Contains**: Lowercase function names, incorrect types, wrong brace style
- **Expected Behavior**: Should trigger warnings when AMIGA mode is enabled

## Test Script

### `test_codex.sh`
A comprehensive Amiga shell script that tests all validation modes:

1. **Basic Usage**: Tests help and error handling
2. **C89 Validation**: Tests C89 compliance checking
3. **SAS/C Compatibility**: Tests detection of VBCC keywords for SAS/C compatibility
4. **VBCC Compatibility**: Tests detection of SAS/C keywords for VBCC compatibility
5. **Amiga Standards**: Tests Amiga coding standards enforcement
6. **Multiple Modes**: Tests combination of multiple validation modes
7. **NDK Validation**: Tests NDK-specific validation
8. **All Modes**: Tests all validation modes simultaneously
9. **Universal Keywords**: Tests universal keyword syntax enforcement
10. **Wildcard Matching**: Tests file pattern matching
11. **Disable Options**: Tests disabling specific validations

## Expected Behavior

### Compiler Compatibility Checks
- **SAS/C mode**: Should flag VBCC-specific keywords as compatibility issues
- **VBCC mode**: Should flag SAS/C-specific keywords as compatibility issues
- **Universal syntax** (e.g., `__SAVE_DS__`): Should NOT trigger warnings in any mode

### C89 Compliance
- Should flag variable declarations not at start of blocks
- Should flag missing return statements in non-void functions
- Should flag other C89 violations

### NDK Mode (Universal Keywords)
- Should flag compiler-specific keywords (e.g., `__saveds`, `__asm`)
- Should suggest universal replacements (e.g., `__SAVE_DS__`, `__ASM__`)
- Should help ensure cross-compiler compatibility via compiler-specific.h

### Amiga Standards
- Should flag lowercase function names (should be PascalCase)
- Should flag incorrect type usage (should use Amiga types)
- Should flag incorrect brace style (should use Allman style)

### Multiple Mode Combination
- Multiple validation modes should work together
- Total issue count should reflect all active validations
- Disable options should override specific validations

## Running the Tests

```bash
# Make the test script executable
chmod +x test_codex.sh

# Run the complete test suite
./test_codex.sh

# Or run individual tests
./Codex test_sasc_specific.c VBCC
./Codex test_vbcc_specific.c SASC
./Codex test_c89_violations.c C89
./Codex test_amiga_standards.c AMIGA
./Codex test_example.c NDK
```

## Key Concept

**Compiler compatibility checks work in the opposite direction of what might seem intuitive:**
- When checking SAS/C compatibility, Codex flags keywords that are NOT compatible with SAS/C
- When checking VBCC compatibility, Codex flags keywords that are NOT compatible with VBCC
- This helps developers identify code that won't compile with their target compiler

The goal is to ensure code can be compiled with the specified compiler, not to validate that it uses that compiler's specific features.

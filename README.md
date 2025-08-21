# Codex

Codex is a comprehensive C linter and style checker designed specifically for Amiga development. It provides static analysis of C source code without compilation, offering immediate feedback on potential issues, style violations, and compatibility problems.

## About Codex

Codex is a static analysis tool that examines C source code for various types of issues. Unlike a compiler, it doesn't generate object code but performs lexical analysis and pattern matching to identify potential problems. This makes it extremely fast and useful for pre-commit validation, continuous integration, learning, and legacy code analysis.

### Key Features

- **Syntax and Standards Compliance** - Ensures your code follows C89 or C99 standards
- **Compiler Compatibility** - Identifies code that won't compile with specific Amiga compilers (SAS/C, VBCC, DICE)
- **Amiga Coding Standards** - Enforces Amiga-specific best practices and conventions
- **Memory Safety** - Detects potentially unsafe functions and suggests alternatives
- **Code Style** - Checks for common style violations like magic numbers and line length

## Operating Modes

### Command Line Usage
Codex follows the standard Amiga command line format:

```bash
# Basic Usage
Codex FILES/M/A,AMIGA/S,NDK/S,C89/S,C99/S,SASC/S,VBCC/S,DICE/S,MEMSAFE/S,QUIET/S,HELP/S

# File Specifications
Codex main.c utils.c

# Command Line Options
C89/S       - Enforce ANSI C89 standards (default)
C99/S       - Enforce C99 standards
AMIGA/S     - Enforce AmigaOS best practices (types, PascalCase). Implies C89 & NDK
MEMSAFE/S   - Check for use of memory-unsafe functions (e.g., strcpy)
NDK/S       - Check for non-universal NDK keywords
SASC/S      - Check for SAS/C keyword compatibility. Implies C89
VBCC/S      - Check for VBCC keyword compatibility. Implies C99
DICE/S      - Check for DICE keyword compatibility. Implies C89 & NDK
QUIET/S     - Suppress summary and only output violation lines
HELP/S      - Display help message

# Examples
Codex MyProject/main.c AMIGA
Codex main.c C99 VBCC
Codex main.c MEMSAFE QUIET
```

## Validation Modes

Codex operates in different validation modes, each focusing on specific aspects of code quality:

### C89 Standards Mode (default)
Enforces strict ANSI C89 compliance, flagging C99 features like `//` comments, `inline` keyword, and variable declarations after statements.

### C99 Standards Mode
Validates against the C99 standard, providing warnings for C99-specific features to ensure the developer is aware they are using them.

### Amiga Standards Mode
Enforces Amiga-specific conventions, such as using Amiga types (`LONG`, `STRPTR`), enforcing `PascalCase` for function names, and using the `NULL` constant.

### Compiler Modes
These modes check for keyword compatibility with a specific target compiler and suggest universal replacements from `<clib/compiler-specific.h>` from the latest Amiga NDK.

### Memory Safety Mode
Identifies potentially memory-unsafe functions from the standard C library, such as `strcpy` and `gets`, and provides guidance on replacing with memory safe versions.

## What Codex Checks

### General Style Checks (Always Active)
- **Line Length** - Flags lines longer than 256 characters
- **Magic Numbers** - Flags hardcoded numerical constants and suggests using named constants

### C89 and C99 Standards Modes
- **Declaration Placement** - Flags variable declarations that appear after a statement within a block
- **C99 Keywords** - Flags C99-specific keywords like `inline`, `restrict`, and `_Bool`
- **C++ Comments** - Flags single-line `//` comments (unless in SASC mode)
- **For-Loop Declarations** - Flags variable declarations inside a `for` loop initializer
- **C99 Features** - Flags designated initializers, compound literals, variadic macros, and flexible array members

### Amiga Standards Mode
- **Amiga Types** - Recommends using Amiga-specific types (`LONG`, `STRPTR`, `APTR`, etc.) instead of standard C types
- **PascalCase** - Checks that user-defined function names use `PascalCase`
- **NULL for Pointers** - Checks for assignments of `0` to pointers and recommends using the `NULL` constant
- **Deprecated Types** - Flags obsolete Amiga types like `USHORT` and `COUNT`

### Memory Safety Mode
- **Unsafe Functions** - Flags a comprehensive list of standard C functions known to be memory-unsafe, including buffer overflow prone functions like `strcpy`, `strcat`, `sprintf` and `gets`

## Building from Source

### Requirements
- SAS/C compiler 6.58
- NDK 3.2
- ToolKit

### Build Commands
```bash
; Using SMakefile
cd Source/
smake Codex
smake install ;Will copy Codex to the SDK/C drawer in the project directory
```

## Installation

1. Find the Codex executable and matching icon in SDK/C/ in this distribution

## ChangeLog

### Version 47.3 (21.08.2025)

- Fixed: Replaced placeholder stdlib functions with SAS/C sc.lib built-ins
- Fixed: Refactored code as a result of the advice received from... Codex
- Fixed: Replaced magic numbers with named constants

### Version 47.2 (20.08.2025)
- Fixed: C89 rules were still checked in C99 mode

### Version 47.1 (20.08.2025)
- Alpha release


## About [amigazen project](http://www.amigazen.com)

*A web, suddenly*

*Forty years meditation*

*Minds awaken, free*

**amigazen project** uses modern software development tools and methods to update and rerelease classic Amiga open source software. Our upcoming releases include a new AWeb, and a new Amiga Python 2.

Key to our approach is ensuring every project can be built with the same common set of development tools and configurations, so we created the ToolKit project to provide a standard configuration for Amiga development. All *amigazen project* releases will be guaranteed to build against the ToolKit standard so that anyone can download and begin contributing straightaway without having to tailor the toolchain for their own setup.

Our philosophy is based on openness:

*Open* to anyone and everyone	- *Open* source and free for all	- *Open* your mind and create!

PRs for all of our projects are gratefully received at [GitHub](https://github.com/amigazen/). While our focus now is on classic 68k software, we do intend that all amigazen project releases can be ported to other Amiga-like systems including AROS and MorphOS where feasible.

## About ToolKit

**ToolKit** exists to solve the problem that most Amiga software was written in the 1980s and 90s, by individuals working alone, each with their own preferred setup for where their dev tools are run from, where their include files, static libs and other toolchain artifacts could be found, which versions they used and which custom modifications they made. Open source collaboration did not exist then as we know it now. 

**ToolKit** from amigazen project is a work in progress to make a standardised installation of not just the Native Developer Kit, but the compilers, build tools and third party components needed to be able to consistently build projects in collaboration with others, without each contributor having to change build files to work with their particular toolchain configuration. 

All *amigazen project* releases will release in a ready to build configuration according to the ToolKit standard.

Each component of **ToolKit** is open source and like *Codex* here will have it's own github repo, while ToolKit itself will be released at https://github.com/amigazen/ToolKit as an easy to install package containing the redistributable components, as well as scripts to easily install the parts that are not freely redistributable from archive.

## Contact 

- At GitHub https://github.com/amigazen/Codex
- on the web at http://www.amigazen.com/toolkit/ (Amiga browser compatible)
- or email toolkit@amigazen.com

## Acknowledgements

*Amiga* is a trademark of **Amiga Inc**. 


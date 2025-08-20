/*
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

/*
 * Test file for C89 compliance functionality.
 * This should trigger warnings when checking C89 standards.
 */

 #include <dos/dos.h>
 #include <proto/dos.h>
 #include <string.h> /* For strcpy */
 
 /* --- C89 Standard Violations --- */
 
 void test_c89_declaration_placement(void)
 {
     int x = 10;
     /* This is a statement, not a declaration */
     Printf("Value of x is %ld\n", x);
 
     /* $CODEX: Variable declaration after a statement is not allowed in C89. */
     int y = 20; /* This declaration after the Printf statement is a C89 violation. */
 
     Printf("Value of y is %ld\n", y);
 }
 
 void test_c99_features_in_c89_mode(void)
 {
     /* $CODEX: C++ style comments ('//') are not allowed in C89. */
     // This is a C99-style comment.
 
     /* $CODEX: Variable declaration in for loop not allowed in C89. */
     for (int i = 0; i < 5; i++) {
         Printf("i = %ld\n", i);
     }
 
     /* $CODEX: C99 designated initializer found - not available in C89. */
     struct Point { int x, y; };
     struct Point p = { .x = 1, .y = 2 };
 }
 
 
 /* --- Amiga Standard Violations --- */
 
 /* $CODEX: Use PascalCase function names. */
 void test_amiga_naming_convention(void)
 {
     /* $CODEX: Use Amiga types (LONG) instead of long. */
     long my_long_var = 12345L;
 
     /* $CODEX: Use Amiga types (UWORD) instead of unsigned short. */
     unsigned short my_ushort = 100;
 
     Printf("Amiga style violations here.\n");
 }
 
 
 /* --- Memory Safety Violations --- */
 
 void test_memsafe_violations(void)
 {
     char buffer[10];
 
     /* $CODEX: Memory-unsafe function 'strcpy' found - use 'strncpy' instead. */
     strcpy(buffer, "This string is definitely too long for the buffer");
 
     Printf("Buffer content: %s\n", buffer);
 }
 
 
 /* --- C89 Compliant Code (Should NOT trigger C89 errors) --- */
 
 void c89_compliant_code(void)
 {
     /* All variables are correctly declared at the top of the block. */
     int x, y, z;
     int i;
 
     x = 10;
     y = 20;
     z = x + y;
 
     /* It is valid to declare a variable at the start of a new, nested block. */
     for (i = 0; i < 5; i++) {
         int j = i * 2; /* This is C89 compliant. */
         Printf("z = %ld, j = %ld\n", z, j);
     }
 }
 
 int main(void)
 {
     Printf("--- Running Codex Tests ---\n");
 
     test_c89_declaration_placement();
     test_c99_features_in_c89_mode();
     test_amiga_naming_convention();
     test_memsafe_violations();
     c89_compliant_code();
 
     Printf("--- Tests Complete ---\n");
 
     return 0;
 }
 
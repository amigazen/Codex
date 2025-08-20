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
 * Test file for MEMSAFE mode functionality
 * This should trigger warnings when checking memory safety.
 */

 #include <dos/dos.h>
 #include <proto/dos.h>
 #include <stdio.h>  /* Required for gets, fgets, stdin, etc. */
 #include <stdlib.h> /* Required for realpath */
 #include <string.h> /* Required for strcpy, strcat, etc. */
 
 /* Memory-unsafe functions that should trigger warnings */
 void memsafe_violation1(void)
 {
     char buffer[256];
     char *src = "Hello World";
 
     /* $CODEX: Memory-unsafe function 'strcpy' found - use 'strncpy' instead. */
     strcpy(buffer, src);
     /* $CODEX: Memory-unsafe function 'strcat' found - use 'strncat' instead. */
     strcat(buffer, " more text");
     /* $CODEX: Memory-unsafe function 'sprintf' found - use 'snprintf' instead. */
     sprintf(buffer, "%s", src);
 }
 
 void memsafe_violation2(void)
 {
     char buffer[256]; /* This was missing */
     char *filename;
     char *path;
 
     /* $CODEX: Memory-unsafe function 'tmpnam' found - use 'tmpnam_r' instead. */
     filename = tmpnam(NULL);
     /* $CODEX: Memory-unsafe function 'realpath' found - use 'realpath' with a pre-allocated buffer instead. */
     path = realpath("/tmp", NULL);
     /* $CODEX: Memory-unsafe function 'gets' found - use 'fgets' instead. */
     gets(buffer);
 }
 
 void memsafe_violation3(void)
 {
     char str[] = "hello,world,test";
     char *token;
 
     /* $CODEX: Memory-unsafe function 'strtok' found - use 'strtok_r' instead. */
     token = strtok(str, ",");
 }
 
 void memsafe_violation4(void)
 {
     int num;
     /* $CODEX: Memory-unsafe function 'scanf' found - use 'check_return_and_width' instead. */
     scanf("%d", &num);
     /* $CODEX: Memory-unsafe function 'sscanf' found - use 'check_return_and_width' instead. */
     sscanf("12345", "%d", &num);
 }
 
 
 /* Memory-safe code that should NOT trigger warnings */
 void memsafe_compliant1(void)
 {
     char buffer[256];
     char *src = "Hello World";
 
     /* Memory-safe alternatives */
     strncpy(buffer, src, sizeof(buffer) - 1);
     buffer[sizeof(buffer) - 1] = '\0';
 
     strncat(buffer, " more text", sizeof(buffer) - strlen(buffer) - 1);
     /* The line below is commented out because snprintf is a C99 function
        and would cause a C89 error, preventing memsafe checks from running. */
     /* snprintf(buffer, sizeof(buffer), "%s", src); */
 }
 
 void memsafe_compliant2(void)
 {
     char buffer[256]; /* This was missing */
     char path[256];
 
     /* Safe alternatives */
     realpath("/tmp", path); /* Correct safe usage */
     fgets(buffer, sizeof(buffer), stdin);
 }
 
 void memsafe_compliant3(void)
 {
     char str[] = "hello,world,test";
     char *token;
     char *saveptr;
 
     /* Safe string tokenization. Commented out because strtok_r is not C89. */
     /* token = strtok_r(str, ",", &saveptr); */
 }
 
 void memsafe_compliant4(void)
 {
     int num;
     int result;
 
     /* Safer scanf requires checking the return value and using width specifiers */
     result = scanf("%10d", &num);
     if (result == 1) {
         /* Success */
     }
 }
 
 int main(void)
 {
     memsafe_violation1();
     memsafe_violation2();
     memsafe_violation3();
     memsafe_violation4();
 
     memsafe_compliant1();
     memsafe_compliant2();
     memsafe_compliant3();
     memsafe_compliant4();
 
     return 0;
 }
 
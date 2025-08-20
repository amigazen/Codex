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
 * Test file for header inclusion functionality.
 * This tests various header inclusion patterns and standards.
 */

#include <dos/dos.h>
#include <proto/dos.h>

/* C89 headers - should be allowed in both C89 and C99 modes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <locale.h>
#include <setjmp.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <float.h>

/* C99+ headers - should trigger warnings in C89 mode */
#include <stdint.h> /* $CODEX: This should trigger a warning: C99+ header not allowed in C89 mode */
#include <stdbool.h> /* $CODEX: This should trigger a warning: C99+ header not allowed in C89 mode */
#include <complex.h> /* $CODEX: This should trigger a warning: C99+ header not allowed in C89 mode */
#include <tgmath.h> /* $CODEX: This should trigger a warning: C99+ header not allowed in C89 mode */
#include <fenv.h> /* $CODEX: This should trigger a warning: C99+ header not allowed in C89 mode */
#include <inttypes.h> /* $CODEX: This should trigger a warning: C99+ header not allowed in C89 mode */
#include <wchar.h> /* $CODEX: This should trigger a warning: C99+ header not allowed in C89 mode */
#include <wctype.h> /* $CODEX: This should trigger a warning: C99+ header not allowed in C89 mode */
#include <uchar.h> /* $CODEX: This should trigger a warning: C99+ header not allowed in C89 mode */
#include <threads.h> /* $CODEX: This should trigger a warning: C99+ header not allowed in C89 mode */
#include <stdatomic.h> /* $CODEX: This should trigger a warning: C99+ header not allowed in C89 mode */
#include <stdnoreturn.h> /* $CODEX: This should trigger a warning: C99+ header not allowed in C89 mode */
#include <stdalign.h> /* $CODEX: This should trigger a warning: C99+ header not allowed in C89 mode */
#include <stdbit.h> /* $CODEX: This should trigger a warning: C99+ header not allowed in C89 mode */

/* Function to test header usage */
void test_headers(void)
{
    /* C89 types and functions */
    int x = 42;
    char str[] = "Hello";
    printf("C89: %d %s\n", x, str);
    
    /* C99+ types and functions */
    bool flag = true; /* $CODEX: This should trigger a warning: C99+ type not allowed in C89 mode */
    int32_t value = 100; /* $CODEX: This should trigger a warning: C99+ type not allowed in C89 mode */
    printf("C99: %d %d\n", flag, value);
}

int main(void)
{
    test_headers();
    return 0;
}

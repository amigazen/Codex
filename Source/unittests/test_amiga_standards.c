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
 * Test file for Amiga standards compliance functionality.
 * This should trigger warnings when checking Amiga coding standards.
 */

#include <dos/dos.h>
#include <proto/dos.h>

/* Amiga standards violations that should trigger warnings */
void amiga_violation1(void)
{
    /* Using lowercase function names instead of PascalCase */
    printf("This should trigger a warning"); /* $CODEX: This should trigger a warning: Use PascalCase function names */
    strcpy(buffer, "string"); /* $CODEX: This should trigger a warning: Use PascalCase function names */
    malloc(100); /* $CODEX: This should trigger a warning: Use PascalCase function names */
}

void amiga_violation2(void)
{
    /* Using lowercase types instead of Amiga types */
    char *ptr; /* $CODEX: This should trigger a warning: Use Amiga types (UBYTE*) instead of char* */
    long value; /* $CODEX: This should trigger a warning: Use Amiga types (LONG) instead of long */
    int counter; /* $CODEX: This should trigger a warning: Use Amiga types (ULONG) instead of int */
    
    /* Should use UBYTE*, LONG, ULONG instead */
}

void amiga_violation3(void)
{
    /* Incorrect brace style (K&R instead of Allman) */
    if (condition) { /* $CODEX: This should trigger a warning: Opening brace should be on its own line (Allman style) */
        /* Do something */
    }
    
    for (int i = 0; i < 10; i++) { /* $CODEX: This should trigger a warning: Opening brace should be on its own line (Allman style) */
        /* Loop body */
    }
}

/* Amiga compliant code that should NOT trigger warnings */
void AmigaCompliant1(void)
{
    /* Using PascalCase function names */
    Printf("This is correct");
    Strncpy(buffer, "string", 10);
    AllocMem(100, MEMF_ANY);
}

void AmigaCompliant2(void)
{
    /* Using Amiga types */
    UBYTE *ptr;
    LONG value;
    ULONG counter;
}

void AmigaCompliant3(void)
{
    /* Correct Allman brace style */
    if (condition)
    {
        /* Do something */
    }
    
    for (int i = 0; i < 10; i++)
    {
        /* Loop body */
    }
}

int main(void)
{
    amiga_violation1();
    amiga_violation2();
    amiga_violation3();
    
    AmigaCompliant1();
    AmigaCompliant2();
    AmigaCompliant3();
    
    return 0;
}

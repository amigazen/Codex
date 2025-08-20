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
 * Example test file for Codex functionality.
 * This demonstrates various coding standards and practices.
 */

#include <dos/dos.h>
#include <proto/dos.h>

/* Compiler-specific keywords that will trigger warnings */
__saveds void test_function1(void) /* $CODEX: This should trigger a warning: Use __SAVE_DS__ for universal syntax */
{
    /* This will trigger a compiler compatibility warning */
    __asm("nop"); /* $CODEX: This should trigger a warning: Use __ASM__ for universal syntax */
}

__save_ds void test_function2(void) /* $CODEX: This should trigger a warning: Use __SAVE_DS__ for universal syntax */
{
    /* Another variant that will trigger a warning */
    __reg("d0", int value) = 42; /* $CODEX: This should trigger a warning: Use __REG__ for universal syntax */
}

__stdargs void test_function3(void) /* $CODEX: This should trigger a warning: Use __STDARGS__ for universal syntax */
{
    /* Function with stdargs calling convention */
    __far char *ptr = NULL; /* $CODEX: This should trigger a warning: Use __FAR__ for universal syntax */
}

__interrupt void test_interrupt_handler(void) /* $CODEX: This should trigger a warning: Use __INTERRUPT__ for universal syntax */
{
    /* Interrupt handler with compiler-specific keyword */
    __chip char buffer[256]; /* $CODEX: This should trigger a warning: Use __CHIP__ for universal syntax */
    __fast int counter = 0; /* $CODEX: This should trigger a warning: Use __FAST__ for universal syntax */
}

__amigainterrupt void test_amiga_interrupt_handler(void) /* $CODEX: This should trigger a warning: Use __INTERRUPT__ for universal syntax */
{
    /* VBCC Amiga-specific interrupt handler */
    __chip char buffer[128]; /* $CODEX: This should trigger a warning: Use __CHIP__ for universal syntax */
}

/* Universal syntax examples (these will NOT trigger warnings) */
__SAVE_DS__ void good_function1(void)
{
    /* Using universal syntax - no warnings */
    __ASM__("nop");
}

__STDARGS__ void good_function2(void)
{
    /* Using universal syntax - no warnings */
    __REG__("d0", int value) = 42;
}

__FAR__ void good_function3(void)
{
    /* Using universal syntax - no warnings */
    __CHIP__ char buffer[256];
    __FAST__ int counter = 0;
}

/* Main function */
int main(void)
{
    /* Test function calls */
    test_function1();
    test_function2();
    test_function3();
    test_interrupt_handler();
    test_amiga_interrupt_handler();
    
    good_function1();
    good_function2();
    good_function3();
    
    return 0;
}

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
 * Test file for SAS/C specific functionality.
 * This tests SAS/C compiler compatibility features.
 */

#include <dos/dos.h>
#include <proto/dos.h>

/* SAS/C specific keywords that should trigger VBCC compatibility warnings */
__saveds void sasc_function1(void) /* $CODEX: This should trigger a warning: Use __SAVE_DS__ for universal syntax */
{
    /* SAS/C calling convention */
    __asm("nop"); /* $CODEX: This should trigger a warning: Use __ASM__ for universal syntax */
}

__save_ds void sasc_function2(void) /* $CODEX: This should trigger a warning: Use __SAVE_DS__ for universal syntax */
{
    /* SAS/C data segment saving */
    __reg("d0", int value) = 42; /* $CODEX: This should trigger a warning: Use __REG__ for universal syntax */
}

__stdargs void sasc_function3(void) /* $CODEX: This should trigger a warning: Use __STDARGS__ for universal syntax */
{
    /* SAS/C standard argument passing */
    __far char *ptr = NULL; /* $CODEX: This should trigger a warning: Use __FAR__ for universal syntax */
}

__interrupt void sasc_interrupt_handler(void) /* $CODEX: This should trigger a warning: Use __INTERRUPT__ for universal syntax */
{
    /* SAS/C interrupt handler */
    __chip char buffer[256]; /* $CODEX: This should trigger a warning: Use __CHIP__ for universal syntax */
    __fast int counter = 0; /* $CODEX: This should trigger a warning: Use __FAST__ for universal syntax */
}

__amigainterrupt void sasc_amiga_interrupt_handler(void) /* $CODEX: This should trigger a warning: Use __INTERRUPT__ for universal syntax */
{
    /* VBCC Amiga-specific interrupt handler (should flag in SAS/C mode) */
    __chip char buffer[128]; /* $CODEX: This should trigger a warning: Use __CHIP__ for universal syntax */
}

/* Universal syntax that should NOT trigger warnings */
__SAVE_DS__ void universal_function1(void)
{
    /* Using universal syntax */
    __ASM__("nop");
}

__STDARGS__ void universal_function2(void)
{
    /* Using universal syntax */
    __REG__("d0", int value) = 42;
}

int main(void)
{
    sasc_function1();
    sasc_function2();
    sasc_function3();
    sasc_interrupt_handler();
    sasc_amiga_interrupt_handler();
    
    universal_function1();
    universal_function2();
    
    return 0;
}

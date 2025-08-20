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
 * Test file for VBCC specific functionality.
 * This tests VBCC compiler compatibility features.
 */

#include <dos/dos.h>
#include <proto/dos.h>

/* VBCC specific keywords that should trigger SAS/C compatibility warnings */
__asm void vbcc_function1(void) /* $CODEX: This should trigger a warning: Use __ASM__ for universal syntax */
{
    /* VBCC inline assembly */
    __reg("d0", int value) = 42; /* $CODEX: This should trigger a warning: Use __REG__ for universal syntax */
}

__interrupt void vbcc_interrupt_handler(void) /* $CODEX: This should trigger a warning: Use __INTERRUPT__ for universal syntax */
{
    /* VBCC interrupt handler */
    __chip char buffer[256]; /* $CODEX: This should trigger a warning: Use __CHIP__ for universal syntax */
    __fast int counter = 0; /* $CODEX: This should trigger a warning: Use __FAST__ for universal syntax */
}

__amigainterrupt void vbcc_amiga_interrupt_handler(void) /* $CODEX: This should trigger a warning: Use __INTERRUPT__ for universal syntax */
{
    /* VBCC Amiga-specific interrupt handler */
    __chip char buffer[128]; /* $CODEX: This should trigger a warning: Use __CHIP__ for universal syntax */
}

__stdargs void vbcc_function2(void) /* $CODEX: This should trigger a warning: Use __STDARGS__ for universal syntax */
{
    /* VBCC standard argument passing */
    __far char *ptr = NULL; /* $CODEX: This should trigger a warning: Use __FAR__ for universal syntax */
}

__saveds void vbcc_function3(void) /* $CODEX: This should trigger a warning: Use __SAVE_DS__ for universal syntax */
{
    /* VBCC calling convention */
    __asm("nop"); /* $CODEX: This should trigger a warning: Use __ASM__ for universal syntax */
}

/* Universal syntax that should NOT trigger warnings */
__ASM__ void universal_function1(void)
{
    /* Using universal syntax */
    __REG__("d0", int value) = 42;
}

__INTERRUPT__ void universal_function2(void)
{
    /* Using universal syntax */
    __CHIP__ char buffer[256];
    __FAST__ int counter = 0;
}

int main(void)
{
    vbcc_function1();
    vbcc_interrupt_handler();
    vbcc_amiga_interrupt_handler();
    vbcc_function2();
    vbcc_function3();
    
    universal_function1();
    universal_function2();
    
    return 0;
}

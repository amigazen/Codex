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
 * Test file for compiler keyword compatibility functionality.
 * This should trigger warnings when checking different compiler compatibilities.
 */

 #include <dos/dos.h>
 #include <proto/dos.h>
 
 /* --- Test Functions --- */
 
 /* SAS/C specific keywords - should flag in VBCC mode */
 __saveds void test_sasc_keywords(void)
 {
     __asm("nop");
     __reg("d0") int value = 42;
 }
 
 /* Another SAS/C specific keyword */
 __save_ds void another_sasc_keyword_test(void)
 {
     Printf("Testing __save_ds\n");
 }
 
 
 /* VBCC specific keywords - should flag in SAS/C mode */
 __amigainterrupt void test_vbcc_keywords(void)
 {
     /* This function definition itself is the test. */
 }
 
 
 /* DICE specific keywords - should flag in both SAS/C and VBCC modes */
 __stkargs void test_dice_keywords(void)
 {
     /* This function definition itself is the test. */
 }
 
 
 /* GCC specific keywords - should flag in both SAS/C and VBCC modes */
 void test_gcc_keywords(void)
 {
     int condition = 1;
 
     struct __attribute__((packed)) gcc_struct
     {
         char a;
         int b;
     };
 
     if (__builtin_expect(condition, 0)) {
         Printf("GCC specific keyword used.\n");
     }
 }
 
 
 /* Universal syntax - should NOT flag in any mode */
 __SAVE_DS__ void test_universal_syntax(void)
 {
     __ASM__("nop");
     __REG__("d0") int value = 42;
     Printf("This function uses universal syntax and should not trigger warnings. Value: %ld\n", value);
 }
 
 
 /* --- Main Entry Point --- */
 
 int main(void)
 {
     /* Call functions to ensure they are part of the compiled code,
        though the linter checks them statically anyway. */
     test_sasc_keywords();
     another_sasc_keyword_test();
     test_vbcc_keywords();
     test_dice_keywords();
     test_gcc_keywords();
     test_universal_syntax();
 
     return 0;
 }
 
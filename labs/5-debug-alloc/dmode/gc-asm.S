#include "rpi-asm.h"

@ for gc mark-and-sweep, we need to also
@ push all registers onto stack so we can check them for pointers
@ to heap objects, then call mark_inner with uint32_t *sp as argument

.extern mark_inner

MK_FN(mark)
    push {r0-r12, lr} @ save all registers
    mov r0, sp @ pass current stack pointer to mark_inner
    bl mark_inner
    pop {r0-r12, pc} @ restore registers and return


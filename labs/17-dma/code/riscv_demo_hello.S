.globl _start

_start:
        la s2, my_string
loop:
        lbu t0, 0(s2)
        beqz t0, end
        addi s2, s2, 1
        jal ra, uart_putc
        j loop
exit:
        .insn 0x0000000b

uart_putc:
        li s0, 0x7e215040
        li s1, 0x7e215064
stat_loop:
        lw t4, 0(s1)
        andi t4, t4, 2
        beqz t4, stat_loop
        sw t0, 0(s0)
        jr ra

my_string:
        .asciz "Hello from DMA!\r\n"

.section .text.start
.globl _start
_start:
    la sp, stack_top
    call reference_main
    li a7, 10
    ecall
.section .bss
.balign 16
stack_bottom:
    .zero 1024
stack_top:

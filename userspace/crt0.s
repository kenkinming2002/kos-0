global _start
extern main

STACK_SIZE equ 4096

section .bss
stack:
  resb STACK_SIZE

section .text
_start:
  mov esp, stack + STACK_SIZE
  push ebp
  mov ebp, esp
  call main
  mov esp, ebp
  pop ebp
  .L1: jmp .L1


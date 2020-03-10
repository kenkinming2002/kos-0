extern _start

section .text

mov esp, stack + STACK_SIZE
mov ebp, stack + STACK_SIZE
call _start

.stop:
  jmp .stop

section .bss
STACK_SIZE equ 256
stack:
  resb STACK_SIZE


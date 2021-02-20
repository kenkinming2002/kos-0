global loader
global kernel_stack_bottom
global kernel_stack_top

extern kmain

STACK_SIZE equ 4096

section .bss
align 4096
kernel_stack_bottom:
  resb STACK_SIZE
kernel_stack_top:

section .text
loader:
  xor ebp, ebp
  mov esp, kernel_stack_top
  push ebx
  call kmain

.fail:
  hlt
  jmp .fail

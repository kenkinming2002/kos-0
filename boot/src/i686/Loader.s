global loader
global kernel_stack_bottom
global kernel_stack_top

extern bmain

STACK_SIZE equ 4096

section .bss
align 4096
kernel_stack_bottom:
  resb STACK_SIZE
kernel_stack_top:

section .text
loader:
  cmp eax, 0x36d76289
  jne .fail

  mov esp, kernel_stack_top

  push ebx
  mov eax, bmain
  call eax

.fail:
  hlt
  jmp .fail

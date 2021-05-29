global _start

section .text
; Loop 1
_start:
  mov esp, stack_top
  mov eax, 0x100
.L0:
  push eax

  mov eax, 1

  mov ecx, esp
  mov edx, .L1
  sysenter
  .L1:

  pop eax

  dec eax
  test eax, eax
  jmp .L0

.L2:
  jmp .L2

section .bss
stack:
  resb 512
stack_top:


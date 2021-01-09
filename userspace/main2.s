global _start

section .text

; Loop 1
_start:
  mov eax, 0x100
.L0:
  mov [stack], eax

  mov eax, 1
  mov edx, .L1
  sysenter
  .L1:

  mov eax, [stack]

  dec eax
  test eax, eax
  jmp .L0

.L2:
  jmp .L2

section .bss
stack:
  resb 1


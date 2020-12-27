section .text

; Loop 1
mov eax, 0x100
.L0:
  int 0x80

  dec eax
  test eax, eax
  jmp .L0

.L2:
  jmp .L2



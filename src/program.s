bits 32

mov eax, command
mov ecx, esp
mov edx, ret
sysenter
ret:

mov eax, 0xdeadbeef
jmp $

string:
  db "Hello world"

align 4
command:
  db 0x4 ; WRITE Command
  db 0x0
  db 0x0
  db 0x0

  db 0x0; IMMEDIATE Operand
  db 0x0
  db 0x0
  db 0x0

  dd string

  db 0x0; IMMEDIATE Operand
  db 0x0
  db 0x0
  db 0x0

  dd 11

  db 0x0; END Command
  db 0x0
  db 0x0
  db 0x0


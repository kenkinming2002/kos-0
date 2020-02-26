bits 32

mov ecx, esp
mov edx, ret

mov eax, 0x3

mov ebx, string
mov esi, 11

sysenter
ret:

mov eax, 0xdeadbeef
jmp $

string:
  db "Hello world"


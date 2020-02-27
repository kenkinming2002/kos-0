bits 32

; Write Hello World
mov ecx, esp
mov edx, ret0

mov eax, 0x3

mov ebx, string
mov esi, 12

sysenter
ret0:

; Yield
mov ecx, esp
mov edx, ret1

mov eax, 0x4

sysenter
ret1:

mov eax, 0xdeadbeef
jmp $

string:
  db `Hello world\n`


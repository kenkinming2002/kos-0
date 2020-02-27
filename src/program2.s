bits 32

mov edi, 0x5

begin:
dec edi

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

test edi, edi
jnz begin

jmp ret0

string:
  db `PROG2 world\n`


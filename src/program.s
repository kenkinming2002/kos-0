bits 32

mov ecx, esp
mov edx, ret
sysenter
ret:

mov eax, 0xdeadbeef
jmp $

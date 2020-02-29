bits 32

mov esp, stack + STACK_SIZE
mov ebp, stack + STACK_SIZE


; Write Hello World
push DWORD length
push string
call write
add esp, 8


call yield

; Write Hello World
push DWORD length
push string
call write
add esp, 8

.L0: 
jmp .L0


write:
  push ebp
  mov ebp, esp

  mov ecx, esp
  mov edx, .ret

  mov eax, 0x3

  mov ebx, [ebp+8]
  mov esi, [ebp+12]

  sysenter
.ret: 

  mov esp, ebp
  pop ebp
  ret

yield:
  push ebp
  mov ebp, esp

  mov ecx, esp
  mov edx, .ret
  
  mov eax, 0x4
  
  sysenter
.ret:

  mov esp, ebp
  pop ebp
  ret


STACK_SIZE equ 256
stack:
  times STACK_SIZE db 0

string:
  db `PROG2 world\n`
  length equ $-string

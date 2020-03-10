global syscall_0
global syscall_1
global syscall_2
global syscall_3
global syscall_4

syscall_0:
  push ebp
  mov ebp, esp

; eax, ecx and edx are coller saved
  mov ecx, esp
  mov edx, .ret
  
  mov eax, [ebp+8] ; syscall number
  
  sysenter
.ret:

  mov esp, ebp
  pop ebp
  ret

syscall_1:
  push ebp
  mov ebp, esp

  push ebx

  mov ecx, esp
  mov edx, .ret
  
  mov eax, [ebp+8] ; syscall number
  mov ebx, [ebp+12]
  
  sysenter
.ret:

  pop ebx

  mov esp, ebp
  pop ebp
  ret

syscall_2:
  push ebp
  mov ebp, esp

  push ebx
  push ebp

  mov ecx, esp
  mov edx, .ret
  
  mov eax, [ebp+8] ; syscall number
  mov ebx, [ebp+12]
  mov ebp, [ebp+16]
  
  sysenter
.ret:

  pop ebp
  pop ebx

  mov esp, ebp
  pop ebp
  ret

syscall_3:
  push ebp
  mov ebp, esp

  push ebx
  push ebp
  push esi

  mov ecx, esp
  mov edx, .ret
  
  mov eax, [ebp+8] ; syscall number
  mov ebx, [ebp+12]
  mov esi, [ebp+20]

  mov ebp, [ebp+16]
  
  sysenter
.ret:

  pop esi
  pop ebp
  pop ebx

  mov esp, ebp
  pop ebp
  ret

syscall_4:
  push ebp
  mov ebp, esp

  push ebx
  push ebp
  push esi
  push edi

  mov ecx, esp
  mov edx, .ret
  
  mov eax, [ebp+8] ; syscall number
  mov ebx, [ebp+12]
  mov esi, [ebp+20]
  mov edi, [ebp+24]

  mov ebp, [ebp+16]
  
  sysenter
.ret:

  pop edi
  pop esi
  pop ebp
  pop ebx

  mov esp, ebp
  pop ebp
  ret

;global yield
;global create_service
;global destroy_service
;global locate_service
;
;global send
;global recv
;
;global write

;yield:
;  push ebp
;  mov ebp, esp
;
;  mov ecx, esp
;  mov edx, .ret
;  
;  mov eax, 0x0
;  
;  sysenter
;.ret:
;
;  mov esp, ebp
;  pop ebp
;  ret
;
;create_service:
;  push ebp
;  mov ebp, esp
;
;  mov ecx, esp
;  mov edx, .ret
;
;  mov eax, 0x1
;
;  mov ebx, [ebp+8]
;
;  sysenter
;.ret: 
;
;  mov esp, ebp
;  pop ebp
;  ret
;
;destroy_service:
;  push ebp
;  mov ebp, esp
;
;  mov ecx, esp
;  mov edx, .ret
;
;  mov eax, 0x2
;
;  mov ebx, [ebp+8]
;
;  sysenter
;.ret: 
;
;  mov esp, ebp
;  pop ebp
;  ret
;
;locate_service:
;  push ebp
;  mov ebp, esp
;
;  mov ecx, esp
;  mov edx, .ret
;
;  mov eax, 0x3
;
;  mov ebx, [ebp+8]
;
;  sysenter
;.ret: 
;
;  mov esp, ebp
;  pop ebp
;  ret
;
;send:
;  push ebp
;  mov ebp, esp
;
;  push ebp
;
;  mov ecx, esp
;  mov edx, .ret
;
;  mov eax, 0x4
;
;  mov ebx, [ebp+8]
;  mov esi, [ebp+16]
;  mov ebp, [ebp+12]
;
;  sysenter
;.ret: 
;
;  pop ebp
;
;  mov esp, ebp
;  pop ebp
;  ret
;
;recv:
;  push ebp
;  mov ebp, esp
;
;  push ebp
;
;  mov ecx, esp
;  mov edx, .ret
;
;  mov eax, 0x5
;
;  mov ebx, [ebp+8]
;  mov esi, [ebp+16]
;  mov ebp, [ebp+12]
;
;  sysenter
;.ret: 
;
;  pop ebp
;
;  mov esp, ebp
;  pop ebp
;  ret
;
;write:
;  push ebp
;  mov ebp, esp
;
;  mov ecx, esp
;  mov edx, .ret
;
;  mov eax, 0x8
;
;  mov ebx, [ebp+8]
;  mov esi, [ebp+12]
;  mov esi, [ebp+16]
;
;  sysenter
;.ret: 
;
;  mov esp, ebp
;  pop ebp
;  ret

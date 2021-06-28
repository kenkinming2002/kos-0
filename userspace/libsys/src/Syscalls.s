global syscall

; [ebp+0x20] - arg 6
; [ebp+0x1C] - arg 5
; [ebp+0x18] - arg 4
; [ebp+0x14] - arg 3
; [ebp+0x10] - arg 2
; [ebp+0x0C] - arg 1
; [ebp+0x08] - syscall no.
; [ebp+0x04] - ebp
; [ebp+0x00] - eip
syscall:
  push ebp
  mov ebp, esp

  push ebx
  push esi
  push edi

  mov eax, [ebp+0x08]

  mov ebx, [ebp+0x0C]
  mov esi, [ebp+0x10]
  mov edi, [ebp+0x14]

; Push in reverse order
  push DWORD [ebp+0x20]
  push DWORD [ebp+0x1C]
  push DWORD [ebp+0x18]

  mov ecx, esp
  mov edx, .L1

  sysenter

.L1:
  add esp, 0xC

  pop edi
  pop esi
  pop ebx

  mov esp, ebp
  pop ebp
  ret

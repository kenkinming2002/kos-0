global core_syscalls_entry
extern core_syscalls_dispatch

; eax - syscall no.
; ebx - arg 1
; ecx - userspace esp
; edx - userspace eip
; esi - arg 2
; edi - arg 3
; ebp - user stack

; TODO: Sanitize kernel registers

core_syscalls_entry:
  push ecx
  push edx

  push ebp
  xor ebp, ebp ; Clear the base pointer to make stack tracing works

  push ecx
  push edi
  push esi
  push ebx
  push eax

  mov bx, 0x10
  mov ds, bx
  mov es, bx
  mov fs, bx
  mov gs, bx

  call core_syscalls_dispatch

  mov bx, 0x23
  mov ds, bx
  mov es, bx
  mov fs, bx
  mov gs, bx

  add esp, 0x14

  pop ebp
  pop edx
  pop ecx

  sti
  sysexit

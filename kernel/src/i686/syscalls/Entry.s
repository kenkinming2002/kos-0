global core_syscalls_entry
extern core_syscalls_dispatch

; TODO: Sanitize kernel registers
core_syscalls_entry:
  push edx ; userspace esp
  push ecx ; userspace eip

  push ebp
  push edi
  push esi
  push edx
  push ecx
  push ebx
  push eax

  mov bx, 0x10
  mov ds, bx
  mov es, bx
  mov fs, bx
  mov gs, bx

  push DWORD 0 ; For stack tracing

  lea eax, [esp+0x4]
  push eax
  call core_syscalls_dispatch

  add esp, 0x8

  mov bx, 0x23
  mov ds, bx
  mov es, bx
  mov fs, bx
  mov gs, bx

  add esp, 0x4; syscall return value
  pop ebx
  add esp, 0x8; ecx, edx
  pop esi
  pop edi
  pop ebp

  pop ecx
  pop edx

  sti
  sysexit

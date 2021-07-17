global core_tasks_entry

struc Registers
  .eax: resd 1
  .ebx: resd 1
  .ecx: resd 1
  .edx: resd 1
  .esi: resd 1
  .edi: resd 1
  .ebp: resd 1
  .esp: resd 1
  .eip: resd 1
endstruc

core_tasks_entry:
  mov bx, 0x23
  mov ds, bx
  mov es, bx
  mov fs, bx
  mov gs, bx

  ; That is the only spare register since
  ; esp and eip would be set to ecx and edx on sysexit
  ; but eip is always used.
  mov esp, [esp+0x4]

  mov eax, [esp+Registers.eax]
  mov ebx, [esp+Registers.ebx]
  mov ecx, [esp+Registers.esp]
  mov edx, [esp+Registers.eip]
  mov esi, [esp+Registers.esi]
  mov edi, [esp+Registers.edi]
  mov ebp, [esp+Registers.ebp]

  sti
  sysexit

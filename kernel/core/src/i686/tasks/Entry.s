global core_tasks_entry

core_tasks_entry:
  mov bx, 0x23
  mov ds, bx
  mov es, bx
  mov fs, bx
  mov gs, bx

  mov ecx, 0
  mov edx, [esp+0x4]

  sti
  sysexit

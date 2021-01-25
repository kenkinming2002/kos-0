global core_tasks_switch_esp

core_tasks_switch_esp:
  push ebx
  push esi
  push edi
  push ebp
  call .L1
  pop ebp
  pop edi
  pop esi
  pop ebx
  ret

; At this point our stack look like so
; [esp+0x1C] - pointer to next     task's esp
; [esp+0x18] - pointer to previous task's esp
; [esp+0x14] - saved eip
; [esp+0x10] - saved ebx
; [esp+0x0C] - saved esi
; [esp+0x08] - saved edi
; [esp+0x04] - saved ebp
; [esp]      - saved eip
.L1:
  mov eax, [esp+0x18]
  mov [eax], esp
  mov eax, [esp+0x1C]
  mov esp, [eax]
  ret

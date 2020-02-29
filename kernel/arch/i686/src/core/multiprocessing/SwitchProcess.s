global switch_process

struc MemoryMapping
  .pageDirectory               : resd 1
  .pageDirectoryPhysicalAddress: resd 1
endstruc

struc ProcessContext
  .esp          : resd 1
  .memoryMapping: resb MemoryMapping_size
endstruc

struc Process
  .context : resb ProcessContext_size
endstruc

; switch_process(Process* previousProcess, Process* nextProcess)
; [esp+8] : nextProcess
; [esp+4] : previousProcess
; [esp  ] : return address

switch_process:
  push ebx
  push esi
  push edi
  push ebp

  ; Save PreviousProcess Stack
  mov edi, [esp+4+4*4]; 4 more registers pushed on the stack
  mov [edi+Process.context + ProcessContext.esp], esp

  ; Load next Task Stack
  mov esi, [esp+8+4*4]
  mov esp, [esi+Process.context + ProcessContext.esp]

  mov eax, [esi+Process.context + ProcessContext.memoryMapping + MemoryMapping.pageDirectoryPhysicalAddress]
  mov cr3, eax

  pop esi
  pop edi
  pop ebx
  pop ebp

  ret

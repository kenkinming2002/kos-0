global enter_user_mode

struc State
  .edi: resd 1
  .esi: resd 1
  .ebp: resd 1
  .esp: resd 1
  .ebx: resd 1
  .edx: resd 1
  .ecx: resd 1
  .eax: resd 1
  .eip: resd 1
endstruc


; [[noreturn]] enter_user_mode(State state)
enter_user_mode:
  cli

  push ebp
  mov ebp, esp

  mov ax, 0x23
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax


  ; Be careful, all addressing must be relative to esp to use the stack segment
  ; register instead of data segment register
  ;
  ; stack: [ebp + 8] State state
  ;        [ebp + 4] return address
  ;        [ebp    ] old ebp

  push 0x23
  push DWORD [esp + 8 + State.esp]

  pushf
  pop eax
  or eax, 0x200 ; Restore IFlag
  push eax

  push 0x1B
  push DWORD [ebp + 8 + State.eip]

  mov eax, [ebp + 8 + State.eax]
  mov ebx, [ebp + 8 + State.ebx]
  mov ecx, [ebp + 8 + State.ecx]
  mov edx, [ebp + 8 + State.edx]

  mov esi, [ebp + 8 + State.esi]
  mov edi, [ebp + 8 + State.edi]
  mov ebp, [ebp + 8 + State.ebp]

  iret

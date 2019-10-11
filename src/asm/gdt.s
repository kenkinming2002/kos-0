global lgdt
global gdt_flush

; lgdt - Loads the global descriptor table
; stack: [esp + 4] The address to the global descriptor table
;        [esp    ] The return address
lgdt:
  mov eax, [esp+4]
  lgdt [eax]
  ret

; gdt_flush - Load the segment register
gdt_flush:
  mov ax, 0x10
  mov ds, ax
  mov ss, ax
  mov es, ax

  jmp 0x08:flush_cs
  flush_cs:

  ret

extern entry

section .ap_trampoline

BITS 16
ap_start16:
  cli
  cld
  xor    ax, ax
  mov    ds, ax
  lgdt   [.gdt_descriptor]
  mov    eax, cr0
  or     eax, 0x1
  mov    cr0, eax
  jmp    0x8:ap_start32

; Note: The osdev tutorial titled Symmetric Multiptrocessing has a bug in
;       that the data segment descriptor is wrong as it does not set
;       D/B(Default operation size) bits, which is set in both the code and tss
;       descriptor.
align 16
.gdt:
  dd 0x0
  dd 0x0

  ; flat code
  dd 0x0000FFFF
  dd 0x00CF9A00

  ; flat data
  dd 0x0000FFFF
  dd 0x00CF9200

  ; tss
  dd 0x00000068
  dd 0x00CF8900

gdt_length equ $ - .gdt

.gdt_descriptor:
  dw gdt_length - 1
  dd .gdt
  dd 0
  dd 0

extern APRunning
extern APCanContinue

BITS 32
ap_start32:
  mov    ax, 16
  mov    ds, ax
  mov    ss, ax

  lock inc BYTE [APRunning]

.wait:
  pause
  cmp BYTE [APCanContinue], 0
  jz  .wait

  jmp entry

.failed:
  jmp .failed


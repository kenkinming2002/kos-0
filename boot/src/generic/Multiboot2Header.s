section .multiboot2_header

; constant
MAGIC_NUMBER  equ 0xE85250D6
ARCHITECTURE  equ 0x00000000
HEADER_LENGTH equ multiboot2_header_length
CHECKSUM      equ -(MAGIC_NUMBER + ARCHITECTURE + HEADER_LENGTH)

; MULTIBOOT HEADER
multiboot2_header:

align 8 ; magic number for boot loader
  dd MAGIC_NUMBER
  dd ARCHITECTURE
  dd HEADER_LENGTH
  dd CHECKSUM

; Note: each tag has to be 8 byte aligned

  dw 6
  dw 0
  dd 8 ; module alignment tag

  dw 1
  dw 0
  dd 12
  dd 6 ; mmap information request
  dd 0 ; Padding

  dw 0
  dw 0
  dd 8 ; end tag

multiboot2_header_length equ $-multiboot2_header

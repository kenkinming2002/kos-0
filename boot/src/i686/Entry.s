global entry

; This is what we need to boot to the real kernel
extern pageDirectory
extern bootInformation
extern entryAddress

; Sychronize me with Config.hpp
PHYSICAL_MEMORY_MAPPING_START equ 0xD0000000;
PHYSICAL_MEMORY_MAPPING_END   equ 0xF0000000;
PHYSICAL_MEMORY_MAPPING_SIZE  equ PHYSICAL_MEMORY_MAPPING_END - PHYSICAL_MEMORY_MAPPING_START;

section .bss
mutex: resd 1

section .text:
entry:
  ; Load page directory
  mov eax, DWORD [pageDirectory]
  mov cr3, eax

  ; Set PSE(Page Size Extension) bit
  mov eax, cr4
  or  eax, 0x00000010
  mov cr4, eax

  ; Set PG(Paging) and PE(Protection) bit
  mov eax, cr0
  or  eax, 0x80000001
  mov cr0, eax

  ; Boot information
  mov eax, DWORD [bootInformation]
  add eax, PHYSICAL_MEMORY_MAPPING_START

  jmp [entryAddress]

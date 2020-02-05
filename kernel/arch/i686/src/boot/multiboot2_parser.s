struc MemmoryMapEntry
  .addr resd 1
  .len  resd 1
  .type resw 1
  .zero resw 1
endstruc

global memory_map_entries
memory_map_entries:
  resb 256 * MemmoryMapEntry_size

global multiboot2_parse

section .text
multiboot2_parse:
  ret

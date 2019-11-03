global loader
extern kmain

; size of kernel stack
KERNEL_STACK_SIZE equ 4096 

; size of page directory - 1024 entries of 4 bytes each
BOOT_PAGE_DIRECTORY_SIZE equ 4096 
; size of page table - 1024 entries of 4 bytes each
BOOT_PAGE_TABLE_SIZE equ 4096 
BOOT_PAGE_TABLE_COUNT equ 1 ; TODO: set this depending on kernel end

section .bss
; Reserve space for kernel stack
align 4
kernel_stack:
  resb KERNEL_STACK_SIZE 

; Reserve space for paging structure
align 4096
boot_page_directory:
  resb BOOT_PAGE_DIRECTORY_SIZE

boot_page_table:
  resb BOOT_PAGE_TABLE_SIZE * BOOT_PAGE_TABLE_COUNT

; ENTRY POINT of our loader
section .text
loader:

%define physical_address(addr) ((addr)-0xC0000000)

; Identity Map the kernel(Must Perserve ebx)
mov ecx, 0
.loop_page_directory:

  mov edx, 0
  .loop_page_table:

  mov esi, ecx
  shl esi, 10
  add esi, edx
  lea edi, [esi * 4 + physical_address(boot_page_table)] ; load physical address to page table entry
  shl esi, 12                                            ; load physical address of page to map
  or esi, 0x003                                          ; set present and writable bit
  mov [edi], esi

  inc edx
  cmp edx, 1024
  jne .loop_page_table

; Map the page table to both 0x00000000 and 0xC0000000
mov esi, ecx
shl esi, 12
add esi, physical_address(boot_page_table)                             ; load physical address of page table into esi
or  esi, 0x003
mov [ecx * 4 + physical_address(boot_page_directory)            ], esi
mov [ecx * 4 + physical_address(boot_page_directory) + (768 * 4)], esi

inc ecx
cmp ecx, BOOT_PAGE_TABLE_COUNT
jne .loop_page_directory

; Enable paging
mov ecx, physical_address(boot_page_directory)
mov cr3, ecx

mov ecx, cr0
or ecx, 0x80010000
mov cr0, ecx

; Enter higher half
lea edx, [higher_half]
jmp edx

higher_half:

; Paging is fully enabled, discard the identity mapping to lower half
mov ecx, 0
.loop_page_directory:

mov [ecx * 4 + boot_page_directory], DWORD 0

inc ecx
cmp ecx, BOOT_PAGE_TABLE_COUNT
jne .loop_page_directory

; Force reload of TLB
mov ecx, cr3
mov cr3, ecx

mov esp, kernel_stack + KERNEL_STACK_SIZE ; point esp to top of kernel stack

push ebx
call kmain

.fail:
  hlt
  jmp .fail

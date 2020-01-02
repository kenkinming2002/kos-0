global loader
extern _start

extern lower_half_main
extern higher_half_main

global boot_page_directory
global boot_page_tables

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

boot_page_tables:
  resb BOOT_PAGE_TABLE_SIZE * BOOT_PAGE_TABLE_COUNT

%define physical_address(addr) ((addr)-0xC0000000)


; ENTRY POINT of our loader
section .boot.text
loader:

; Lower Half Main
mov esp, physical_address(kernel_stack + KERNEL_STACK_SIZE) ; point esp to top of kernel stack

push ebx
mov eax, lower_half_main
call eax

; Call into Higher Half
lea eax, [higher_half]
call eax

section .text

higher_half:

; Higher Half Main
mov esp, kernel_stack + KERNEL_STACK_SIZE ; point esp to top of kernel stack

mov eax, higher_half_main
call eax

; Actual Main
call _start

.fail:
  hlt
  jmp .fail

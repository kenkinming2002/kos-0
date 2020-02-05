global loader
extern _start

extern lower_half_main
extern higher_half_main

; size of kernel stack
KERNEL_STACK_SIZE equ 4096 

section .bss
; Reserve space for kernel stack
align 4
kernel_stack:
  resb KERNEL_STACK_SIZE 

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

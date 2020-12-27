global loader
global bootInformationStorageBegin
global bootInformationStorageEnd
global kernel_stack_bottom
global kernel_stack_top

extern lower_half_main
extern higher_half_main
extern _start

%define physical_address(addr) ((addr)-0xC0000000)

PAGE_SIZE equ 4096 

;==============
; BSS section ;
;==============
section .bss
align 4096
bootInformationStorageBegin:
  resb PAGE_SIZE
bootInformationStorageEnd:

align 4096
kernel_stack_bottom:
  resb PAGE_SIZE
kernel_stack_top:

;==================
; Lower Half Main ;
;==================
section .boot.text
loader:
  cmp eax, 0x36d76289
  jne .fail

  mov esp, physical_address(kernel_stack_top)

  push ebx
  mov eax, lower_half_main
  call eax

;========================
; Call into Higher Half ;
;========================
  lea eax, [higher_half]
  call eax

.fail:
  hlt
  jmp .fail

;===================
; Higher Half Main ;
;===================
section .text
higher_half:
  xor ebp, ebp
  mov esp, kernel_stack_top

  mov eax, higher_half_main
  call eax

  ; Actual Main
  call _start

.fail:
  hlt
  jmp .fail

global loader

extern kmain

; Set up at most 16 stacks each 4k
STACK_SIZE equ 4096
STACK_SHIFT equ 12
STACK_COUNT equ 16

section .bss
align 4096
bootstrap_stack:
  resb STACK_SIZE * STACK_COUNT

coresCount: resb 1

; When we get here, the register holds the following value
; eax = pageDirectory = cr3
; ebx = bootInformation
; ecx = entryAddress = loader
; edx = coresCount

; eax = bootInformation
; ebx = APIC id

section .text
loader:
; Get ACPI id
  mov esi, eax
  mov eax, 0x1
  cpuid
  mov eax, esi
  shr ebx, 0x18

; Sanity check
  cmp ebx, STACK_COUNT
  jae .fail

; Setup stack
  mov ecx, ebx
  shl ecx, STACK_SHIFT;

  xor ebp, ebp
  mov esp, bootstrap_stack + STACK_SIZE
  add esp, ecx

; Run
  push ebx
  push eax
  call kmain

.fail:
  hlt
  jmp .fail

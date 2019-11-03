#ifndef ASM_IDT_H
#define ASM_IDT_H

#ifdef __cplusplus
extern "C" {
#endif

void lidt(const void* idt_descriptor);

#ifdef __cplusplus
}
#endif

#endif // ASM_IDT_H

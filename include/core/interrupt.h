#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

typedef struct idt_entry_ia32
{
  uint16_t offset_low; 
  uint16_t selector; 
  uint8_t  zero;      
  uint8_t  type_attr; 
  uint16_t offset_high; 
} __attribute((packed)) idt_entry_t;

enum irq_type
{
  IRQ32_TASK_GATE = 0x05,
  IRQ16_INTERRUPT_GATE = 0x6,
  IRQ16_TRAP_GATE = 0x7,

  IRQ32_INTERRUPT_GATE = 0xE,
  IRQ32_TRAP_GATE = 0xF
};

enum irq_privillege_level
{
  IRQ_RING0 = 0,
  IRQ_RING1 = 1,
  IRQ_RING2 = 2,
  IRQ_RING3 = 3
};

typedef void *irq_handler_t;

/** idt_init_entry Initialize an entry in interrupt descriptor table
 *
 *  @param idt_entry            Pointer to idt_entry structure to initialize
 *  @param irq_type             Type of interrupt handler
 *  @param irq_privillege_level Privillege level interrupt handler execute in
 *
 *  @return 0 on success, a negative value otherwise
 */
int idt_init_entry(idt_entry_t *idt_entry, enum irq_type irq_type, enum irq_privillege_level irq_privillege_level, 
    uint16_t selector, irq_handler_t irq_handler);

/** idt_update Update the interrupt descriptor table pointed to by interrupt
 *             descriptor table register
 *
 *  @param idt_entries array of 256 interrupt descriptor table entries
 *
 *  @return 0 on success, a negative value otherwise
 */
int idt_update(idt_entry_t (*idt_entries)[256]);

#endif // INTERRUPT_H

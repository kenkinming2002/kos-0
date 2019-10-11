#include <core/interrupt.h>

#include <asm/idt.h>

int idt_init_entry(idt_entry_t *idt_entry, enum irq_type irq_type, enum irq_privillege_level irq_privillege_level, 
    uint16_t selector, irq_handler_t irq_handler)
{
  idt_entry->type_attr = irq_type | irq_privillege_level << 5 | 1 << 7;
  idt_entry->selector  = selector;

  idt_entry->offset_low  = (uint32_t)irq_handler & 0xFFFF;
  idt_entry->offset_high = (uint32_t)irq_handler >> 16;

  idt_entry->zero = 0;

  return 0;
}

struct idt
{
  unsigned short size;
  unsigned int   offset;
} __attribute__((packed));

int idt_update(idt_entry_t (*idt_entries)[256])
{
  struct idt idt;
  idt.size = sizeof(*idt_entries) - 1;
  idt.offset = (unsigned int)idt_entries;

  lidt(&idt);

  return 0;
}

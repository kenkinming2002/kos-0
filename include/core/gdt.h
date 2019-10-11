#ifndef CORE_GDT_H
#define CORE_GDT_H

#include <stdint.h>
#include <stddef.h>

typedef struct gdt_entry
{
  unsigned short limit_low;
  unsigned short base_low;

  unsigned char base_middle;
  unsigned char access;

  unsigned char granularity;
  unsigned char base_high;
} __attribute((packed)) gdt_entry_t;

enum privillege_level
{
  RING0 = 0,
  RING1 = 1,
  RING2 = 2,
  RING3 = 3
};

enum segment_type
{
  NONE_SEGMENT        = 0x00,
  DATA_SEGMENT        = 0x92, /*0b10010010*/
  DATA_SEGMENT_RDONLY = 0x90, /*0b10010000*/
  CODE_SEGMENT        = 0x9F, /*0b10011000*/
  CODE_SEGMENT_RD     = 0x9A  /*0b10011010*/
};

/** idt_init_entry Initialize an entry in interrupt descriptor table
 *
 *  @param idt_entry            Pointer to idt_entry structure to initialize
 *  @param irq_type             Type of interrupt handler
 *  @param irq_privillege_level Privillege level interrupt handler execute in
 *
 *  @return 0 on success, a negative value otherwise
 */
int gdt_init_entry(gdt_entry_t *gdt_entry, uint32_t base, uint32_t limit, enum privillege_level privillege_level, 
    enum segment_type segment_type);

/** idt_update Update the interrupt descriptor table pointed to by interrupt
 *             descriptor table register
 *
 *  @param idt_entries pointer to idt entries
 *
 *  @return 0 on success, a negative value otherwise
 */
int gdt_update(gdt_entry_t *gdt_entries, size_t size);

#endif // CORE_GDT_H

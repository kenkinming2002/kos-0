#include <core/gdt.h>

#include <asm/gdt.h>

int gdt_init_entry(gdt_entry_t *gdt_entry, uint32_t base, uint32_t limit, enum privillege_level privillege_level, 
    enum segment_type segment_type)
{
  gdt_entry->base_low    = (base & 0xFFFF);
  gdt_entry->base_middle = (base >> 16) & 0xFF;
  gdt_entry->base_high   = (base >> 24) & 0xFF;

  if(limit>65536)
  {
    if((limit & 0xFFF) != 0xFFF)
      return -1; // some of the last 12 bits are set, which will be discarded

    limit >>= 12;
    gdt_entry->granularity = 0xC0;
  }
  else
    gdt_entry->granularity = 0x40;

  gdt_entry->limit_low    = limit & 0xffff;
  gdt_entry->granularity |= (limit>>16) & 0xF;

  gdt_entry->access = (uint8_t)privillege_level<<5 | (uint8_t)segment_type;

  return 0;
}

struct gdt
{
  unsigned short size;
  unsigned int offset;
} __attribute((packed));

int gdt_update(gdt_entry_t *gdt_entries, size_t size)
{
  struct gdt gdt;
  gdt.size = sizeof(*gdt_entries) * size - 1;
  gdt.offset = (unsigned int)gdt_entries;

  lgdt(&gdt);
  gdt_flush();

  return 0;
}

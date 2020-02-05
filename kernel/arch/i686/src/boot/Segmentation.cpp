#include <i686/boot/Segmentation.hpp>

#include <intel/asm/gdt.hpp>

namespace boot
{
  GDTEntry::GDTEntry(uint32_t base, uint32_t limit, PrivillegeLevel privillegeLevel, SegmentType segmentType)
  {
    m_baseLow    = (base & 0xFFFF);
    m_baseMiddle = (base >> 16) & 0xFF;
    m_baseHigh   = (base >> 24) & 0xFF;

    if(limit>65536)
    {
      // Silently discard all lower bit
      limit >>= 12;
      m_granularity = 0xC0;
    }
    else
      m_granularity = 0x40;

    m_limitLow    = limit & 0xffff;
    m_granularity |= (limit>>16) & 0xF;

    m_access = static_cast<uint8_t>(privillegeLevel)<<5 | static_cast<uint8_t>(segmentType);
  }

  GDT::GDT(const GDTEntry* gdtEntries, size_t size)
  {
    m_size = sizeof (GDTEntry) * size - 1;
    m_offset = reinterpret_cast<uint32_t>(gdtEntries);
  }

  int GDT::load() const
  {
    asm volatile ( R"(
      .intel_syntax noprefix
        lgdt [%[gdt]]
      .att_syntax prefix
      )"
      :
      : [gdt]"r"(this)
      :
    );

    asm volatile ( R"(
      .intel_syntax noprefix
        mov ax, 0x10
        mov ds, ax
        mov ss, ax
        mov es, ax
        jmp 0x08:flush_cs
      flush_cs:
      .att_syntax prefix
      )"
      :
      :
      : "ax"
    );

    return 0;
  }
}

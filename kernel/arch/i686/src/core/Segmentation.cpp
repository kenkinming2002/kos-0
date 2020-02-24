#include <i686/core/Segmentation.hpp>

#include <intel/asm/gdt.hpp>

namespace core
{
  GDTEntry::GDTEntry(uint32_t base, uint32_t limit, PrivillegeLevel privillegeLevel, SegmentType segmentType, Granularity granularity)
  {
    m_baseLow    = (base & 0xFFFF);
    m_baseMiddle = (base >> 16) & 0xFF;
    m_baseHigh   = (base >> 24) & 0xFF;

    if(granularity == Granularity::PAGE)
    {
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
    m_offset = reinterpret_cast<uintptr_t>(gdtEntries);
  }

  int GDT::load() const
  {
    assembly::lgdt(this);
    assembly::gdt_flush();

    return 0;
  }
}

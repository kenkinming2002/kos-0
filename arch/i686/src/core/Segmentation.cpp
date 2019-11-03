#include <i686/core/Segmentation.hpp>

#include <i686/asm/gdt.h>

namespace core::i686
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
    lgdt(this);
    gdt_flush();

    return 0;
  }
}

namespace core
{
  Segmentation::Segmentation() 
    : m_gdtEntries{
        i686::GDTEntry(0, 0,          PrivillegeLevel::RING0, i686::SegmentType::NONE_SEGMENT),
        i686::GDTEntry(0, 0xffffffff, PrivillegeLevel::RING0, i686::SegmentType::CODE_SEGMENT_RD),
        i686::GDTEntry(0, 0xffffffff, PrivillegeLevel::RING0, i686::SegmentType::DATA_SEGMENT)
      }
  {

    i686::GDT(m_gdtEntries, GDT_SIZE).load();
  }
}

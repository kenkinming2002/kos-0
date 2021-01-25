#pragma once

#include <stdint.h>
#include <stddef.h>

#include <i686/PrivilegeLevel.hpp>

namespace core::internals
{
  enum class SegmentType
  {
    NONE_SEGMENT        = 0x00,
    DATA_SEGMENT        = 0x92, /*0b10010010*/
    DATA_SEGMENT_RDONLY = 0x90, /*0b10010000*/
    CODE_SEGMENT        = 0x9F, /*0b10011000*/
    CODE_SEGMENT_RD     = 0x9A, /*0b10011010*/
    TASK_STATE_SEGMENT  = 0x89  /*0b10001001*/
  };

  enum class Granularity
  {
    BYTE,
    PAGE
  };

  class GDTEntry
  {
  public:
    GDTEntry() = default;
    GDTEntry(uint32_t base, uint32_t limit, PrivilegeLevel privilegeLevel, SegmentType segmentType, Granularity granularity);

  private:
    unsigned short m_limitLow    = 0;
    unsigned short m_baseLow     = 0;
    unsigned char  m_baseMiddle  = 0;
    unsigned char  m_access      = 0;
    unsigned char  m_granularity = 0;
    unsigned char  m_baseHigh    = 0;
  } __attribute((packed));

  class GDT
  {
  public:
    GDT(const GDTEntry* gdtEntries, size_t size);

  private:
    uint16_t m_size;
    uint32_t m_offset;
  } __attribute__((packed));
}

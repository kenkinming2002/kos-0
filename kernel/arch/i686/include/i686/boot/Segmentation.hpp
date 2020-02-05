#pragma once

#include <generic/core/PrivillegeLevel.hpp>
#include <i686/core/Segmentation.hpp>

#include <stdint.h>
#include <stddef.h>

#include <boot/boot.hpp>

namespace boot
{
  using core::PrivillegeLevel;

  enum class SegmentType
  {
    NONE_SEGMENT        = 0x00,
    DATA_SEGMENT        = 0x92, /*0b10010010*/
    DATA_SEGMENT_RDONLY = 0x90, /*0b10010000*/
    CODE_SEGMENT        = 0x9F, /*0b10011000*/
    CODE_SEGMENT_RD     = 0x9A  /*0b10011010*/
  };

  class GDTEntry
  {
  public:
    BOOT_FUNCTION GDTEntry() = default;
    BOOT_FUNCTION GDTEntry(uint32_t base, uint32_t limit, PrivillegeLevel privillegeLevel, SegmentType segmentType);

  private:
    unsigned short m_limitLow;
    unsigned short m_baseLow;

    unsigned char m_baseMiddle;
    unsigned char m_access;

    unsigned char m_granularity;
    unsigned char m_baseHigh;
  } __attribute((packed));

  class GDT
  {
  public:
    BOOT_FUNCTION GDT(const GDTEntry* gdtEntries, size_t size);

  public:
    BOOT_FUNCTION int load() const;

  private:
    uint16_t m_size;
    uint32_t m_offset;
  } __attribute__((packed));
}

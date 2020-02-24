#pragma once

#include <generic/core/PrivillegeLevel.hpp>

#include <stdint.h>
#include <stddef.h>

namespace core
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
    GDTEntry(uint32_t base, uint32_t limit, PrivillegeLevel privillegeLevel, SegmentType segmentType, Granularity granularity);

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
    GDT(const GDTEntry* gdtEntries, size_t size);

  public:
    int load() const;

  private:
    uint16_t m_size;
    uint32_t m_offset;
  } __attribute__((packed));

  struct TaskStateSegment
  {
    uint32_t prev_tss;   
    uint32_t esp0;     
    uint32_t ss0;     
    uint32_t esp1;   
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;         
    uint32_t cs;        
    uint32_t ss;        
    uint32_t ds;        
    uint32_t fs;       
    uint32_t gs;         
    uint32_t ldt;      
    uint16_t trap;
    uint16_t iomap_base;
  } __attribute__((packed));
}


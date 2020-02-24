#include <i686/core/Segmentation.hpp>

#include <intel/asm/gdt.hpp>

#include <cstddef>

extern "C"
{
  extern std::byte kernel_stack[];
}

namespace core::segmentation
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

  namespace
  {
    static constexpr size_t GDT_SIZE = 6;
    GDTEntry GDTEntries[GDT_SIZE];
    TaskStateSegment kernelTaskStateSegment;
  }

  void init()
  {
    // None segment in the beginning
    GDTEntries[0] = GDTEntry(0, 0,          PrivillegeLevel::RING0, SegmentType::NONE_SEGMENT, Granularity::PAGE);

    // Kernel segments
    GDTEntries[1] = GDTEntry(0, 0xffffffff, PrivillegeLevel::RING0, SegmentType::CODE_SEGMENT_RD, Granularity::PAGE);
    GDTEntries[2] = GDTEntry(0, 0xffffffff, PrivillegeLevel::RING0, SegmentType::DATA_SEGMENT, Granularity::PAGE);

    // User Segments
    GDTEntries[3] = GDTEntry(0, 0xffffffff, PrivillegeLevel::RING3, SegmentType::CODE_SEGMENT_RD, Granularity::PAGE);
    GDTEntries[4] = GDTEntry(0, 0xffffffff, PrivillegeLevel::RING3, SegmentType::DATA_SEGMENT, Granularity::PAGE);

    // Task State Segment
    GDTEntries[5] = GDTEntry(reinterpret_cast<uintptr_t>(&kernelTaskStateSegment), sizeof(TaskStateSegment), PrivillegeLevel::RING0, SegmentType::TASK_STATE_SEGMENT, Granularity::BYTE);

    GDT(GDTEntries, GDT_SIZE).load();

    // 2: Set up Task State Segment
    kernelTaskStateSegment.ss0 = 0x10;
    kernelTaskStateSegment.esp0 = reinterpret_cast<uintptr_t>(kernel_stack);
    asm volatile ( R"(
      .intel_syntax noprefix
        mov ax, 0x28
        ltr ax
      .att_syntax prefix
      )"
      :
      : 
      : "ax"
    );
  }
}

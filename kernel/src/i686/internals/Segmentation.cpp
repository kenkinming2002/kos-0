#include <i686/internals/Segmentation.hpp>

#include <common/generic/io/Print.hpp>

#include <cstddef>

namespace core::internals
{
  using namespace common;

  static constexpr size_t GDT_SIZE = 6;
  constinit static TSS tss;
  constinit static GDTEntry gdtEntries[GDT_SIZE];

  void initializeSegmentation()
  {
    io::print("Loading Global Descriptor Table and Segment Registers...");

    // Null Segment
    gdtEntries[0] = GDTEntry(0, 0,          PrivilegeLevel::RING0, SegmentType::NONE_SEGMENT, Granularity::PAGE);

    // Kernel Segment
    gdtEntries[1] = GDTEntry(0, 0xffffffff, PrivilegeLevel::RING0, SegmentType::CODE_SEGMENT_RD, Granularity::PAGE);
    gdtEntries[2] = GDTEntry(0, 0xffffffff, PrivilegeLevel::RING0, SegmentType::DATA_SEGMENT   , Granularity::PAGE);

    // Userspace Segment
    gdtEntries[3] = GDTEntry(0, 0xffffffff, PrivilegeLevel::RING3, SegmentType::CODE_SEGMENT_RD, Granularity::PAGE);
    gdtEntries[4] = GDTEntry(0, 0xffffffff, PrivilegeLevel::RING3, SegmentType::DATA_SEGMENT   , Granularity::PAGE);

    // Task State Segment
    gdtEntries[5] = GDTEntry(reinterpret_cast<uintptr_t>(&tss), sizeof tss, PrivilegeLevel::RING0, SegmentType::TASK_STATE_SEGMENT, Granularity::BYTE);

    // Load GDT
    GDT gdt(gdtEntries, sizeof gdtEntries / sizeof gdtEntries[0]);
    asm volatile ( 
      "lgdt %[gdt];" 
      "mov ax, 0x10;"
      "mov ds, ax;"
      "mov ss, ax;"
      "mov es, ax;"
      "jmp 0x08:1f;"
      "1:" 
      "mov ax, 0x28;"
      "ltr ax" 
      : : [gdt]"m"(gdt) : "ax"
    );
    io::print("Done\n");
  }

  void setKernelStack(uint32_t ss, uint32_t esp)
  {
    tss.ss0  = ss;
    tss.esp0 = esp;
  }
}


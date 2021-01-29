#include <i686/internals/Segmentation.hpp>

#include <common/generic/io/Print.hpp>

#include <cstddef>

namespace core::internals
{
  using namespace common;

  constinit TSS tss;
  static auto& gdtEntries()
  {
    static const GDTEntry gdtEntries[] = {
      // Null Segment
      GDTEntry(0, 0,          PrivilegeLevel::RING0, SegmentType::NONE_SEGMENT, Granularity::PAGE),

      // Kernel Segment
      GDTEntry(0, 0xffffffff, PrivilegeLevel::RING0, SegmentType::CODE_SEGMENT_RD, Granularity::PAGE),
      GDTEntry(0, 0xffffffff, PrivilegeLevel::RING0, SegmentType::DATA_SEGMENT   , Granularity::PAGE),

      // Userspace Segment
      GDTEntry(0, 0xffffffff, PrivilegeLevel::RING3, SegmentType::CODE_SEGMENT_RD, Granularity::PAGE),
      GDTEntry(0, 0xffffffff, PrivilegeLevel::RING3, SegmentType::DATA_SEGMENT   , Granularity::PAGE),

      // Task State Segment
      GDTEntry(reinterpret_cast<uintptr_t>(&tss), sizeof tss, PrivilegeLevel::RING0, SegmentType::TASK_STATE_SEGMENT, Granularity::BYTE)
    };
    return gdtEntries;
  }

  void initializeSegmentation()
  {
    io::print("Loading Global Descriptor Table and Segment Registers...");
    // Load gdt
    GDT gdt(gdtEntries(), sizeof gdtEntries() / sizeof gdtEntries()[0]);
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
}


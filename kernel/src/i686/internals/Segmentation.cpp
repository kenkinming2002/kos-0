#include <i686/internals/Segmentation.hpp>

#include <i686/internals/GDT.hpp>
#include <i686/internals/TSS.hpp>

#include <generic/Init.hpp>
#include <generic/PerCPU.hpp>

#include <librt/Log.hpp>
#include <librt/Global.hpp>

#include <stddef.h>

namespace core::internals
{
  static constexpr size_t GDT_SIZE = 6;

  constinit static rt::Global<PerCPU<GDTEntry[GDT_SIZE]>> gdtEntries;
  constinit static rt::Global<PerCPU<TSS>> tss;

  void initializeSegmentation()
  {
    rt::log("Loading Global Descriptor Table and Segment Registers...");

    gdtEntries.construct();
    tss.construct();
    foreachCPUInitCall([](){
      auto& gdtEntriesCurrent = gdtEntries().current();
      auto& tssCurrent        = tss().current();

      // Null Segment
      gdtEntriesCurrent[0] = GDTEntry(0, 0,          PrivilegeLevel::RING0, SegmentType::NONE_SEGMENT, Granularity::PAGE);

      // Kernel Segment
      gdtEntriesCurrent[1] = GDTEntry(0, 0xffffffff, PrivilegeLevel::RING0, SegmentType::CODE_SEGMENT_RD, Granularity::PAGE);
      gdtEntriesCurrent[2] = GDTEntry(0, 0xffffffff, PrivilegeLevel::RING0, SegmentType::DATA_SEGMENT   , Granularity::PAGE);

      // Userspace Segment
      gdtEntriesCurrent[3] = GDTEntry(0, 0xffffffff, PrivilegeLevel::RING3, SegmentType::CODE_SEGMENT_RD, Granularity::PAGE);
      gdtEntriesCurrent[4] = GDTEntry(0, 0xffffffff, PrivilegeLevel::RING3, SegmentType::DATA_SEGMENT   , Granularity::PAGE);

      // Task State Segment
      gdtEntriesCurrent[5] = GDTEntry(reinterpret_cast<uintptr_t>(&tssCurrent), sizeof tssCurrent, PrivilegeLevel::RING0, SegmentType::TASK_STATE_SEGMENT, Granularity::BYTE);

      GDT gdt(gdtEntriesCurrent, sizeof gdtEntriesCurrent / sizeof gdtEntriesCurrent[0]);

      // Load GDT
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
    });
    rt::log("Done\n");
  }

  void setKernelStack(uint32_t ss, uint32_t esp)
  {
    tss().current().ss0  = ss;
    tss().current().esp0 = esp;
  }
}


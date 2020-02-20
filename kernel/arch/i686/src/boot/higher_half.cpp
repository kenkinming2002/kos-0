#include <i686/boot/higher_half.hpp>

#include <i686/boot/boot.hpp>

#include <i686/core/memory/Paging.hpp>
#include <i686/core/Segmentation.hpp>

#include <generic/utils/Utilities.hpp>

extern "C"
{
  extern std::byte kernel_physical_start[];
  extern std::byte kernel_physical_end[];
}

extern "C" void higher_half_main()
{
  // 1: set up segmentation
  auto& GDTEntries = utils::deref_cast<core::GDTEntry[GDT_SIZE]>(kernelGDTEntries);
  // None segment in the beginning
  GDTEntries[0] = core::GDTEntry(0, 0,          core::PrivillegeLevel::RING0, core::SegmentType::NONE_SEGMENT);

  // Kernel segments
  GDTEntries[1] = core::GDTEntry(0, 0xffffffff, core::PrivillegeLevel::RING0, core::SegmentType::CODE_SEGMENT_RD);
  GDTEntries[2] = core::GDTEntry(0, 0xffffffff, core::PrivillegeLevel::RING0, core::SegmentType::DATA_SEGMENT);

  GDTEntries[3] = core::GDTEntry(0, 0xffffffff, core::PrivillegeLevel::RING3, core::SegmentType::CODE_SEGMENT_RD);
  GDTEntries[4] = core::GDTEntry(0, 0xffffffff, core::PrivillegeLevel::RING3, core::SegmentType::DATA_SEGMENT);

  core::GDT(GDTEntries, GDT_SIZE).load();


  // 2: remove unused Page Table
  auto& pageDirectory = utils::deref_cast<core::memory::MemoryMapping>(kernelMemoryMapping).pageDirectory;
  auto& pageTables    = utils::deref_cast<core::memory::PageTable[BOOT_PAGE_TABLE_COUNT]>(kernelPageTable);

  for(size_t pageDirectoryIndex=0; pageDirectoryIndex<BOOT_PAGE_TABLE_COUNT; ++pageDirectoryIndex)
  {
    auto& lowerHalfPageDirectoryEntry  = pageDirectory[pageDirectoryIndex];
    lowerHalfPageDirectoryEntry.present(false);
  }

  for(size_t pageFrameIndex=reinterpret_cast<uintptr_t>(kernel_physical_end) / 4096u; pageFrameIndex<BOOT_PAGE_TABLE_COUNT * core::memory::PAGE_TABLE_ENTRY_COUNT; ++pageFrameIndex)
  {
    auto pageTablesIndex = pageFrameIndex / core::memory::PAGE_TABLE_ENTRY_COUNT;
    auto pageTableIndex  = pageFrameIndex % core::memory::PAGE_TABLE_ENTRY_COUNT;

    auto& pageTableEntry = pageTables[pageTablesIndex][pageTableIndex];
    pageTableEntry.present(false);
  }

  asm volatile ( R"(
    .intel_syntax noprefix
      mov eax, cr3
      mov cr3, eax
    .att_syntax prefix
    )"
    :
    : 
    : "eax"
  );
}

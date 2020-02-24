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
  // 1: remove unused Page Table
  auto& pageDirectory = utils::deref_cast<core::memory::PageDirectory>(kernelPageDirectory);
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

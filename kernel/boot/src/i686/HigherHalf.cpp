#include <boot/i686/HigherHalf.hpp>

#include <boot/i686/Boot.hpp>
#include <common/i686/memory/Paging.hpp>

extern "C"
{
  extern char kernel_physical_start[];
  extern char kernel_physical_end[];
  extern char kernel_boot_section_virtual_begin[];
  extern char kernel_boot_section_virtual_end[];
}

extern "C" void higher_half_main()
{
  using namespace common::memory;

  // 1: Remove lower half mapping
  auto& pageDirectory = *reinterpret_cast<PageDirectory*>(initialPageDirectory);
  auto& pageTables    = *reinterpret_cast<PageTable(*)[BOOT_PAGE_TABLE_COUNT]>(kernelPageTables);

  for(size_t pageDirectoryIndex=0; pageDirectoryIndex<BOOT_PAGE_TABLE_COUNT; ++pageDirectoryIndex)
  {
    auto& lowerHalfPageDirectoryEntry  = pageDirectory[pageDirectoryIndex];
    lowerHalfPageDirectoryEntry = PageDirectoryEntry();
  }

  // 2: Remove mapping in boot section
  for(size_t pageDirectoryIndex=0; pageDirectoryIndex<BOOT_PAGE_TABLE_COUNT; ++pageDirectoryIndex)
  {
    auto& pageTable = pageTables[pageDirectoryIndex];
    for(size_t pageTableIndex=0; pageTableIndex<common::memory::PAGE_TABLE_ENTRY_COUNT; ++pageTableIndex)
    {
      auto& pageTableEntry = pageTable[pageTableIndex];
      uintptr_t physicalAddress = (pageDirectoryIndex * 1024u + pageTableIndex) * 4096u;
      uintptr_t virtualAddress = physicalAddress + 0xC0000000;

      if(virtualAddress>=reinterpret_cast<uintptr_t>(kernel_boot_section_virtual_begin) && virtualAddress < reinterpret_cast<uintptr_t>(kernel_boot_section_virtual_end))
        pageTableEntry = PageTableEntry();
    }
  }

  asm volatile ( "mov eax, cr3; mov cr3, eax " : : : "eax");
}

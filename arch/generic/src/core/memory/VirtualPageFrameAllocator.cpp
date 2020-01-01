#include <generic/core/memory/VirtualPageFrameAllocator.hpp>

#include <algorithm>
#include <tuple>
#include <utils/Utilities.hpp>

namespace core::memory
{
  std::optional<VirtualPageFrameRange> VirtualPageFrameAllocator::getUsableVirtualPageFrameRange(size_t n) const
  {
    auto& pageDirectory = utils::deref_cast<core::PageDirectory>(boot_page_directory);
    for(size_t i=0; i<PAGE_DIRECTORY_ENTRY_COUNT-1/* The last page directory entry is reserverd for fractal mapping*/; ++i)
    {
      auto& pageDirectoryEntry = pageDirectory[i];
      if(!pageDirectoryEntry.present())
      {
        if(n<=PAGE_TABLE_ENTRY_COUNT)
          return VirtualPageFrameRange(i*PAGE_TABLE_ENTRY_COUNT, n);
        else
          return std::nullopt;
      }
      auto& pageTable = utils::deref_cast<core::PageTable>(pageDirectoryEntry.address());
      auto pageTableUsable = [](const core::PageTableEntry& pageTable) { return !pageTable.present(); };
      auto find_range = [](auto begin, auto end, auto predicate) {
        auto begin_range = std::find_if(begin, end, predicate);
        auto end_range = std::find_if_not(begin_range, end, predicate);
        return std::make_tuple(begin_range, end_range);
      };
      for(auto [begin, end] = find_range(std::begin(pageTable), std::end(pageTable), pageTableUsable); 
          end != std::end(pageTable); 
          std::tie(begin, end) = find_range(end, std::end(pageTable), pageTableUsable))

      {
        auto length = std::distance(begin, end);
        if(length>=n)
          return VirtualPageFrameRange(i*PAGE_TABLE_ENTRY_COUNT+begin-std::begin(pageTable), n);
      }
    }
    return std::nullopt;
  }

  VirtualPageFrameAllocator::MapResult VirtualPageFrameAllocator::map(PhysicalPageFrameRange physicalPageFrameRange,
      VirtualPageFrameRange virtualPageFrameRange, std::optional<VirtualPageFrameRange> pageTablePhysicalMemory) const
  {
    auto& pageDirectory = utils::deref_cast<core::PageDirectory>(boot_page_directory);

    // NOTE : index = pageDirectoryIndex * PAGE_TABLE_ENTRY_COUNT +
    //                pageTableIndex
    auto pageDirectoryIndex = virtualPageFrameRange.index / PAGE_TABLE_ENTRY_COUNT;
    auto pageTableIndex     = virtualPageFrameRange.index % PAGE_TABLE_ENTRY_COUNT;

    auto& pageDirectoryEntry = pageDirectory[pageDirectoryIndex];

    uintptr_t pageTablePhysicalAddress;
    if(!pageDirectoryEntry.present())
    {
      if(pageTablePhysicalMemory)
        pageTablePhysicalAddress = reinterpret_cast<uintptr_t>(pageTablePhysicalMemory->toPageFrames());
      else
        return MapResult::ERR_NO_PAGE_TABLE;
    }
    else
      pageTablePhysicalAddress = pageDirectoryEntry.address();

    // 1: Set up Fractal Mapping to Page Table
    uintptr_t alignedPageTablePhysicalAddress = pageTablePhysicalAddress & 0x00400000;
    uintptr_t pageTablePhysicalAddressOffset  = pageTablePhysicalAddress & 0x003FFFFF;

    auto& fractalMappingPageDirectoryEntry = pageDirectory[PAGE_DIRECTORY_ENTRY_COUNT-1]; // Access last Page Directory
    fractalMappingPageDirectoryEntry = core::PageDirectoryEntry(pageTablePhysicalAddress, core::CacheMode::ENABLED, core::WriteMode::WRITE_BACK,
          core::Access::SUPERVISOR_ONLY, core::Permission::READ_WRITE, core::PageSize::LARGE);
    auto fractalMappingPageFrames = VirtualPageFrameRange((PAGE_DIRECTORY_ENTRY_COUNT-1) * PAGE_TABLE_ENTRY_COUNT, PAGE_TABLE_ENTRY_COUNT).toPageFrames();
    auto& pageTable = utils::deref_cast<core::PageTable>(reinterpret_cast<uintptr_t>(fractalMappingPageFrames) + pageTablePhysicalAddressOffset);

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

    // 2: Clear page table to zero if it is new
    if(!pageDirectoryEntry.present())
    {
      for(auto& pageTableEntry: pageTable)
        pageTableEntry = core::PageTableEntry();
    }

    // 2: Write to Page Table through Fractal Mapping
    auto physicalPageFrames = physicalPageFrameRange.toPageFrames();
    for(size_t i = 0; i<virtualPageFrameRange.count; ++i)
    {
      auto physicalPageFrame = &physicalPageFrames[i];
      pageTable[pageTableIndex+i] = core::PageTableEntry(reinterpret_cast<uintptr_t>(physicalPageFrame), core::TLBMode::LOCAL, 
          core::CacheMode::ENABLED, core::WriteMode::WRITE_BACK, core::Access::SUPERVISOR_ONLY, core::Permission::READ_WRITE);
    }

    // 3: Update Page Directory Entry to point to the possibly new Page Table
    if(!pageDirectoryEntry.present())
      pageDirectoryEntry = core::PageDirectoryEntry(pageTablePhysicalAddress, core::CacheMode::ENABLED, core::WriteMode::WRITE_BACK,
            core::Access::SUPERVISOR_ONLY, core::Permission::READ_WRITE);


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

    return MapResult::SUCCESS;
  }
}


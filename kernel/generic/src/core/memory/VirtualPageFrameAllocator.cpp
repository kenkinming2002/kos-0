#include <generic/core/memory/VirtualPageFrameAllocator.hpp>

#include <algorithm>
#include <tuple>
#include <generic/utils/Utilities.hpp>

#include <i686/boot/lower_half.hpp>
#include <generic/io/Print.hpp>

namespace core::memory
{
  VirtualPageFrameAllocator::VirtualPageFrameAllocator(std::byte* begin, std::byte* end)
  {
    m_memoryRegions.push_front(*(new MemoryRegion(begin, end)));
  }

  std::optional<MemoryRegion> VirtualPageFrameAllocator::allocate(size_t count)
  {
    io::print("DEBUG: Allocating Virtual Page Frames\n");
    io::print("  Requested Page Frame Count: ", count, "\n");

    for(MemoryRegion& memoryRegion : m_memoryRegions)
    {
      if(memoryRegion.count()>=count)
      {
        // Remove from the front of a Virtual Page Frame Range
        auto allocatedMemoryRegion = memoryRegion.shrink_front(count);

        // NOTE: This may create page frame with zero size. However, that does
        //       not matter since page frame is cheap(only 0x40 byte on i686
        //       including all control structure i.e liballoc tag and forward
        //       list pointer) and will be reclaimed during merging when we
        //       deallocate. Besides, it is hard to correctly erase the node,
        //       since we may be called from deallocate, which want to allocate
        //       space to hold the freed page. It is possible that after we
        //       return, deallocate will attempt to insert the newly allocated
        //       node after the node we just deleted, which is undefined
        //       behavior. It is not safe to assume that the node we just
        //       deleted may still be used since that may be the last node on an
        //       allocated page, which is subsequently freed and unmapped after
        //       deallocation.
        //
        //       Following is a counter for those who try to implement freeing
        //       of zero-sized page and either failed or given up: 1

        io::print("  Allocated Virtual Frame: ", allocatedMemoryRegion.begin(), "-", allocatedMemoryRegion.end(), "\n");
        return allocatedMemoryRegion;
      }
    }
    
    io::print("  ERROR: No virtual frame available\n");
    return std::nullopt;
  }

  void VirtualPageFrameAllocator::deallocate(MemoryRegion freedMemoryRegion)
  {
    auto before = std::adjacent_find(m_memoryRegions.begin(), m_memoryRegions.end(), [&](auto& /*lhs*/, auto& rhs){
        return freedMemoryRegion < rhs;
    }), after = std::next(before);

    /** At this point the ordering look like so
     *    => *before -> freedMemoryRegion -> *after =>
     */
    if(before->tryMergeAfter(freedMemoryRegion))
    {
      if(before->tryMergeAfter(*after))
      {
        m_memoryRegions.erase_after_and_dispose(before, [](auto* ptr){
            delete ptr;
        });
        return;
      }
    }
    else if(after->tryMergeBefore(freedMemoryRegion))
      return;
    else
      m_memoryRegions.insert_after(before, *(new MemoryRegion(freedMemoryRegion)));
  }

  VirtualPageFrameAllocator::MapResult VirtualPageFrameAllocator::map(MemoryRegion physicalMemoryRegion, MemoryRegion virtualMemoryRegion, void* pageTablePhysicalMemory) const
  {
    // NOTE : index = pageDirectoryIndex * PAGE_TABLE_ENTRY_COUNT +
    //                pageTableIndex
    auto pageDirectoryIndex = virtualMemoryRegion.beginIndex() / PAGE_TABLE_ENTRY_COUNT;
    auto pageTableIndex     = virtualMemoryRegion.beginIndex() % PAGE_TABLE_ENTRY_COUNT;

    auto& pageDirectory = utils::deref_cast<core::PageDirectory>(kernelPageDirectory);
    auto& pageDirectoryEntry = pageDirectory[pageDirectoryIndex];

    // 2: Obtain physical Address to target page table
    phyaddr_t pageTablePhysicalAddress;
    if(!pageDirectoryEntry.present())
    {
      if(pageTablePhysicalMemory)
        pageTablePhysicalAddress = reinterpret_cast<phyaddr_t>(pageTablePhysicalMemory);
      else
        return MapResult::ERR_NO_PAGE_TABLE;
    }
    else
      pageTablePhysicalAddress = pageDirectoryEntry.address();

    auto& pageTable = utils::deref_cast<core::PageTable>(doFractalMapping(pageTablePhysicalAddress));

    // 3: Clear page table to zero if it is new
    if(!pageDirectoryEntry.present())
    {
      for(PageTableEntry& pageTableEntry: pageTable)
        pageTableEntry = core::PageTableEntry();
    }

    // 3: Write to Page Table through Fractal Mapping
    for(size_t i = 0; i<virtualMemoryRegion.count(); ++i)
    {
      //auto physicalPageFrame = &physicalPageFrames[i];
      auto physicalAddress = physicalMemoryRegion.begin() + i * PAGE_SIZE;

      auto pageTableEntry = core::PageTableEntry(physicalAddress, core::TLBMode::LOCAL, core::CacheMode::ENABLED, core::WriteMode::WRITE_BACK, core::Access::SUPERVISOR_ONLY, core::Permission::READ_WRITE);
      if(pageTableIndex+i<1024)
        pageTable[pageTableIndex+i] = pageTableEntry;
      else
        for(;;) asm("hlt"); // Page Table Overrun, Unimplemented
    }

    // 4: Update Page Directory Entry to point to the possibly new Page Table
    if(!pageDirectoryEntry.present())
      pageDirectoryEntry = core::PageDirectoryEntry(pageTablePhysicalAddress, core::CacheMode::ENABLED, core::WriteMode::WRITE_BACK, core::Access::SUPERVISOR_ONLY, core::Permission::READ_WRITE);


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

  MemoryRegion VirtualPageFrameAllocator::unmap(MemoryRegion virtualMemoryRegion) const
  {
    // NOTE : index = pageDirectoryIndex * PAGE_TABLE_ENTRY_COUNT +
    //                pageTableIndex
    auto pageDirectoryIndex = virtualMemoryRegion.beginIndex() / PAGE_TABLE_ENTRY_COUNT;
    auto pageTableIndex     = virtualMemoryRegion.beginIndex() % PAGE_TABLE_ENTRY_COUNT;

    auto& pageDirectory = utils::deref_cast<core::PageDirectory>(kernelPageDirectory);
    auto& pageDirectoryEntry = pageDirectory[pageDirectoryIndex];

    if(!pageDirectoryEntry.present())
        for(;;) asm("hlt"); // WTH, unmapping a region that is never mapped!?

    phyaddr_t pageTablePhysicalAddress = pageDirectoryEntry.address();
    auto& pageTable = utils::deref_cast<core::PageTable>(doFractalMapping(pageTablePhysicalAddress));

    // Take the address mapped by first page table as the starting address for mapped PhysicalPageFrameRange
    auto physicalAddressMapped = pageTable[pageTableIndex].address(); 

    // Unset all Page Table Entry through Fractal Mapping
    for(size_t i = 0; i<virtualMemoryRegion.count(); ++i)
    {
      if(pageTableIndex+i<1024)
      {
        auto& pageTableEntry = pageTable[pageTableIndex+i];
        pageTableEntry.present(false);
        pageTableEntry.address(0u); // For consistency only
      }
      else
        for(;;) asm("hlt"); // Page Table Overrun, Unimplemented
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

    // NOTE: There is no need to free the page table, we can reuse it next time

    return MemoryRegion(physicalAddressMapped / PAGE_SIZE, virtualMemoryRegion.count(), MemoryRegion::index_length_tag);
  }

  virtaddr_t VirtualPageFrameAllocator::doFractalMapping(phyaddr_t physicalAddress) const
  {
    auto& pageDirectory = utils::deref_cast<core::PageDirectory>(kernelPageDirectory);

    // Write to last Page Directory
    uintptr_t alignedPhysicalAddress = physicalAddress & 0xFFC00000; // Aligned to 4MiB mark
    pageDirectory[PAGE_DIRECTORY_ENTRY_COUNT-1] = core::PageDirectoryEntry(alignedPhysicalAddress, core::CacheMode::ENABLED,
          core::WriteMode::WRITE_BACK, core::Access::SUPERVISOR_ONLY, core::Permission::READ_WRITE, core::PageSize::LARGE);

    // Compute Virtual Address for starting of Fractal Mapping
    //auto fractalMappingPageFrameRange = VirtualPageFrameRange((PAGE_DIRECTORY_ENTRY_COUNT-1)
    //        * PAGE_TABLE_ENTRY_COUNT, PAGE_TABLE_ENTRY_COUNT);
    //auto fractalMappingPageFrames = reinterpret_cast<std::byte*>(fractalMappingPageFrameRange.toPageFrames());

    virtaddr_t fractalMappingAddress = (PAGE_DIRECTORY_ENTRY_COUNT-1) * PAGE_TABLE_ENTRY_COUNT * PAGE_SIZE;
    
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

    // Add back the discarded bit due to 4Mib alignment
    return fractalMappingAddress + (physicalAddress & 0x003FFFFF);
  }
}


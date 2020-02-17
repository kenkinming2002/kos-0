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
    auto virtualPageFrameRange = new VirtualPageFrameRange(VirtualPageFrameRange::from_pointers(begin, end));
    m_virtualPageFrameRanges.push_front(*virtualPageFrameRange);
  }

  std::optional<VirtualPageFrameRange> VirtualPageFrameAllocator::allocate(size_t count)
  {
    io::print("DEBUG: Allocating Virtual Page Frames\n");
    io::print("  Requested Page Frame Count: ", count, "\n");

    for(VirtualPageFrameRange& virtualPageFrameRange : m_virtualPageFrameRanges)
    {
      if(virtualPageFrameRange.count>=count)
      {
        // Remove from the front of a Virtual Page Frame Range
        auto allocatedVirtualPageFrameRange = VirtualPageFrameRange(virtualPageFrameRange.index, count);
        virtualPageFrameRange.index += count;
        virtualPageFrameRange.count -= count;

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

        io::print("  Allocated Virtual Frame: ", allocatedVirtualPageFrameRange.begin_index() * VirtualPageFrameRange::SIZE, "-", allocatedVirtualPageFrameRange.end_index() * VirtualPageFrameRange::SIZE, "\n");
        return allocatedVirtualPageFrameRange;
      }
    }
    
    io::print("  ERROR: No virtual frame available\n");
    return std::nullopt;
  }

  void VirtualPageFrameAllocator::deallocate(VirtualPageFrameRange freedVirtualPageFrameRange)
  {
    auto merge_to = [](VirtualPageFrameRange& to, VirtualPageFrameRange& from) -> bool {
      if(from.index + from.count == to.index)
      {
        to.count += from.count;
        to.index = from.index;
        from = VirtualPageFrameRange(0, 0);
        return true;
      }
      else
        return false;
    };
    for(auto outer_it = m_virtualPageFrameRanges.begin(); outer_it!=m_virtualPageFrameRanges.end(); ++outer_it)
    {
      auto& virtualPageFrameRange = *outer_it;

      if(merge_to(virtualPageFrameRange, freedVirtualPageFrameRange))
      {
        for(auto inner_before_it = m_virtualPageFrameRanges.before_begin(), inner_it = m_virtualPageFrameRanges.begin(); 
            inner_it!=outer_it; ++inner_before_it, ++inner_it)
        {
          if(!merge_to(*std::next(inner_it), *inner_it))
          {
            m_virtualPageFrameRanges.erase_after_and_dispose(inner_before_it, [](auto* pageFrameRange){
              delete pageFrameRange;
            });
          }
          else
            break;
        }
      }

      if(freedVirtualPageFrameRange < virtualPageFrameRange)
      {
        auto heapFreedVirtualPageFrameRange = new VirtualPageFrameRange(freedVirtualPageFrameRange);
        return (void)m_virtualPageFrameRanges.insert_after(outer_it, *heapFreedVirtualPageFrameRange);
        return;
      }
    }
  }

  VirtualPageFrameAllocator::MapResult VirtualPageFrameAllocator::map(PhysicalPageFrameRange physicalPageFrameRange,
      VirtualPageFrameRange virtualPageFrameRange, std::optional<VirtualPageFrameRange> pageTablePhysicalMemory) const
  {
    auto& pageDirectory = utils::deref_cast<core::PageDirectory>(bootPageDirectory);

    // NOTE : index = pageDirectoryIndex * PAGE_TABLE_ENTRY_COUNT +
    //                pageTableIndex
    auto pageDirectoryIndex = virtualPageFrameRange.index / PAGE_TABLE_ENTRY_COUNT;
    auto pageTableIndex     = virtualPageFrameRange.index % PAGE_TABLE_ENTRY_COUNT;

    auto& pageDirectoryEntry = pageDirectory[pageDirectoryIndex];

    uintptr_t pageTablePhysicalAddress;
    if(!pageDirectoryEntry.present())
    {
      if(pageTablePhysicalMemory)
      {
        if(pageTablePhysicalMemory->count * VirtualPageFrameRange::SIZE != sizeof(PageTable))
          return MapResult::ERR_INVALID_PAGE_TABLE;

        pageTablePhysicalAddress = reinterpret_cast<uintptr_t>(pageTablePhysicalMemory->toPageFrames());
      }
      else
        return MapResult::ERR_NO_PAGE_TABLE;
    }
    else
      pageTablePhysicalAddress = pageDirectoryEntry.address();

    auto& pageTable = utils::deref_cast<core::PageTable>(doFractalMapping(pageTablePhysicalAddress));

    // 2: Clear page table to zero if it is new
    if(!pageDirectoryEntry.present())
    {
      for(PageTableEntry& pageTableEntry: pageTable)
        pageTableEntry = core::PageTableEntry();
    }

    // 2: Write to Page Table through Fractal Mapping
    auto physicalPageFrames = physicalPageFrameRange.toPageFrames();
    for(size_t i = 0; i<virtualPageFrameRange.count; ++i)
    {
      auto physicalPageFrame = &physicalPageFrames[i];
      auto pageTableEntry = core::PageTableEntry(reinterpret_cast<uintptr_t>(physicalPageFrame), core::TLBMode::LOCAL, 
          core::CacheMode::ENABLED, core::WriteMode::WRITE_BACK, core::Access::SUPERVISOR_ONLY, core::Permission::READ_WRITE);
      pageTable[pageTableIndex+i] = pageTableEntry;
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

  std::optional<PhysicalPageFrameRange> VirtualPageFrameAllocator::unmap(VirtualPageFrameRange virtualPageFrameRange) const
  {
    auto& pageDirectory = utils::deref_cast<core::PageDirectory>(bootPageDirectory);

    // NOTE : index = pageDirectoryIndex * PAGE_TABLE_ENTRY_COUNT +
    //                pageTableIndex
    auto pageDirectoryIndex = virtualPageFrameRange.index / PAGE_TABLE_ENTRY_COUNT;
    auto pageTableIndex     = virtualPageFrameRange.index % PAGE_TABLE_ENTRY_COUNT;

    auto& pageDirectoryEntry = pageDirectory[pageDirectoryIndex];

    if(!pageDirectoryEntry.present())
      return std::nullopt;

    uintptr_t pageTablePhysicalAddress = pageDirectoryEntry.address();
    auto& pageTable = utils::deref_cast<core::PageTable>(doFractalMapping(pageTablePhysicalAddress));

    // Take the address mapped by first page table as the starting address for mapped PhysicalPageFrameRange
    auto physicalAddressMapped = pageTable[pageTableIndex].address(); 

    // Unset all Page Table Entry through Fractal Mapping
    for(size_t i = 0; i<virtualPageFrameRange.count; ++i)
    {
      auto& pageTableEntry = pageTable[pageTableIndex+i];
      pageTableEntry.present(false);
      pageTableEntry.address(0u); // For consistency only
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

    return VirtualPageFrameRange(physicalAddressMapped / VirtualPageFrameRange::SIZE, virtualPageFrameRange.count);
  }

  std::byte* VirtualPageFrameAllocator::doFractalMapping(uintptr_t physicalAddress) const
  {
    auto& pageDirectory = utils::deref_cast<core::PageDirectory>(bootPageDirectory);

    // Write to last Page Directory
    uintptr_t alignedPhysicalAddress = physicalAddress & 0xFFC00000; // Aligned to 4MiB mark
    pageDirectory[PAGE_DIRECTORY_ENTRY_COUNT-1] = core::PageDirectoryEntry(alignedPhysicalAddress, core::CacheMode::ENABLED,
          core::WriteMode::WRITE_BACK, core::Access::SUPERVISOR_ONLY, core::Permission::READ_WRITE, core::PageSize::LARGE);

    // Compute Virtual Address for starting of Fractal Mapping
    auto fractalMappingPageFrameRange = VirtualPageFrameRange((PAGE_DIRECTORY_ENTRY_COUNT-1)
            * PAGE_TABLE_ENTRY_COUNT, PAGE_TABLE_ENTRY_COUNT);
    auto fractalMappingPageFrames = reinterpret_cast<std::byte*>(fractalMappingPageFrameRange.toPageFrames());
    
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
    return fractalMappingPageFrames + (physicalAddress & 0x003FFFFF);
  }
}


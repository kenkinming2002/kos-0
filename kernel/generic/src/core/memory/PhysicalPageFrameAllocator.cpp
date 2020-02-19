#include <generic/core/memory/PhysicalPageFrameAllocator.hpp>

#include <i686/boot/lower_half.hpp>
#include <generic/io/Print.hpp>

#include <algorithm>

extern "C"
{
  extern std::byte kernel_virtual_start[];
  extern std::byte kernel_virtual_end[];

  extern std::byte kernel_physical_start[];
  extern std::byte kernel_physical_end[];

  //extern std::byte early_heap_start[];
  //extern std::byte early_heap_end[];
}

namespace core::memory
{
  PhysicalPageFrameAllocator::PhysicalPageFrameAllocator(struct multiboot_mmap_entry* mmap_entries, size_t length) 
  {
    io::print("DEBUG: Initializing Physical Memory Manager\n");

    io::print("  Kernel physical address range: ", reinterpret_cast<uintptr_t>(kernel_physical_start), "-", reinterpret_cast<uintptr_t>(kernel_physical_end), "\n");
    auto kernelMemoryRegion = MemoryRegion(kernel_physical_start, kernel_physical_end);

    io::print("  Usable physical address ranges(including address range):\n");
    auto it = m_memoryRegions.before_begin();
    for(size_t i=0; i<length; ++i)
    {
      struct multiboot_mmap_entry& mmap_entry = mmap_entries[i];
      if(mmap_entry.type == MULTIBOOT_MEMORY_AVAILABLE)
      {
        io::print("   - ", mmap_entry.addr, "-", mmap_entry.addr + mmap_entry.len, "\n");
        auto [first, second] = MemoryRegion::difference(MemoryRegion(mmap_entry), kernelMemoryRegion);
        {
          if(first)
            it = m_memoryRegions.insert_after(it, *(new MemoryRegion(*first)));
          if(second)
            it = m_memoryRegions.insert_after(it, *(new MemoryRegion(*second)));
        }
      }
    }
  }

  std::optional<MemoryRegion> PhysicalPageFrameAllocator::allocate(size_t count)
  {
    io::print("DEBUG: Allocating Physical Page Frames\n");
    io::print("  Requested Page Frame Count: ", count, "\n");

    for(MemoryRegion& memoryRegion : m_memoryRegions)
    {
      if(memoryRegion.count()>=count)
      {
        // Remove from the front of a Physical Page Frame Range
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

        io::print("  Allocated Physical Frame: ", allocatedMemoryRegion.begin() , "-", allocatedMemoryRegion.end(), "\n");
        return allocatedMemoryRegion;
      }
    }
    
    io::print("  ERROR: No physical frame available\n");
    return std::nullopt;
  }

  void PhysicalPageFrameAllocator::deallocate(MemoryRegion freedMemoryRegion)
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
}


#include <generic/core/memory/PhysicalPageFrameAllocator.hpp>

#include <boot/lower_half.hpp>
#include <io/Print.hpp>

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
    io::print(__PRETTY_FUNCTION__);

    auto kernelPageFrameRange = PhysicalPageFrameRange::from_address(reinterpret_cast<uintptr_t>(kernel_physical_start), 
                                                               kernel_physical_end - kernel_physical_start);

    auto it = m_physicalPageFrameRanges.before_begin();
    for(size_t i=0; i<length; ++i)
    {
      struct multiboot_mmap_entry& mmap_entry = mmap_entries[i];
      if(mmap_entry.type == MULTIBOOT_MEMORY_AVAILABLE)
      {
        auto [first, second] = PhysicalPageFrameRange::from_multiboot_entry(mmap_entry).carve(kernelPageFrameRange);
        {
          it = m_physicalPageFrameRanges.insert_after(it, *(new PhysicalPageFrameRange(first)));
          if(second)
            it = m_physicalPageFrameRanges.insert_after(it, *(new PhysicalPageFrameRange(*second)));
        }
      }
    }
  }

  std::optional<PhysicalPageFrameRange> PhysicalPageFrameAllocator::allocate(size_t count)
  {
    io::print(__PRETTY_FUNCTION__);
    io::print("Requested Page Frame Count:", count);

    for(PhysicalPageFrameRange& physicalPageFrameRange : m_physicalPageFrameRanges)
    {
      if(physicalPageFrameRange.count>=count)
      {
        // Remove from the front of a Physical Page Frame Range
        auto allocatedPhysicalPageFrameRange = PhysicalPageFrameRange(physicalPageFrameRange.index, count);
        physicalPageFrameRange.index += count;
        physicalPageFrameRange.count -= count;

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

        io::print("Allocated Physical Frame - index:", allocatedPhysicalPageFrameRange.index, " count:", allocatedPhysicalPageFrameRange.count);
        io::print("-----FUNCTION EXIT-----");
        return allocatedPhysicalPageFrameRange;
      }
    }
    
    io::print("-----FUNCTION EXIT-----");
    return std::nullopt;
  }

  void PhysicalPageFrameAllocator::deallocate(PhysicalPageFrameRange freedPhysicalPageFrameRange)
  {
    auto merge_to = [](PhysicalPageFrameRange& to, PhysicalPageFrameRange& from) -> bool {
      if(from.index + from.count == to.index)
      {
        to.count += from.count;
        to.index = from.index;
        from = PhysicalPageFrameRange(0, 0);
        return true;
      }
      else
        return false;
    };
    for(auto outer_it = m_physicalPageFrameRanges.begin(); outer_it!=m_physicalPageFrameRanges.end(); ++outer_it)
    {
      auto& physicalPageFrameRange = *outer_it;

      if(merge_to(physicalPageFrameRange, freedPhysicalPageFrameRange))
      {
        for(auto inner_before_it = m_physicalPageFrameRanges.before_begin(), inner_it = m_physicalPageFrameRanges.begin(); 
            inner_it!=outer_it; ++inner_before_it, ++inner_it)
        {
          if(!merge_to(*std::next(inner_it), *inner_it))
            m_physicalPageFrameRanges.erase_after_and_dispose(inner_before_it, [](auto* physicalPageFrameRange){
                delete physicalPageFrameRange;
            });
          else
            break;
        }
      }

      if(freedPhysicalPageFrameRange < physicalPageFrameRange)
      {
        // Insert since merge is impossible
        auto* heapFreedPhysicalPageFrameRange = new PhysicalPageFrameRange(freedPhysicalPageFrameRange);
        return (void)m_physicalPageFrameRanges.insert_after(outer_it, *heapFreedPhysicalPageFrameRange);
      }
    }
  }
}


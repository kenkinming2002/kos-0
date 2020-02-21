#include <generic/core/memory/LinkedListMemoryRegionAllocator.hpp>

#include <algorithm>

namespace core::memory
{
  std::optional<MemoryRegion> LinkedListMemoryRegionAllocator::allocate(size_t count)
  {
    for(MemoryRegion& memoryRegion : m_memoryRegions)
    {
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
      if(memoryRegion.count()>=count)
        return memoryRegion.shrink_front(count);
    }
    return std::nullopt;
  }

  void LinkedListMemoryRegionAllocator::deallocate(MemoryRegion freedMemoryRegion)
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


#include <generic/core/memory/PhysicalPageFrameAllocator.hpp>

#include <boot/lower_half.hpp>

extern "C"
{
  extern char kernel_virtual_start[];
  extern char kernel_virtual_end[];

  extern char kernel_physical_start[];
  extern char kernel_physical_end[];
}

namespace core::memory
{
  //template
  //class PhysicalPageFrameAllocator<utils::ForwardList>;
  //

  template class PhysicalPageFrameAllocator<StaticVector>;

  template<template<typename> typename Container>
  PhysicalPageFrameAllocator<Container>::PhysicalPageFrameAllocator(struct multiboot_mmap_entry* mmap_entries, size_t length)
  {
    auto kernelPageFrameRange = PhysicalPageFrameRange::from_address(reinterpret_cast<uintptr_t>(kernel_physical_start), 
                                                               kernel_physical_end - kernel_physical_start);

    for(size_t i=0; i<length; ++i)
    {
      struct multiboot_mmap_entry& mmap_entry = mmap_entries[i];
      if(mmap_entry.type == MULTIBOOT_MEMORY_AVAILABLE)
      {
        auto [first, second] = PhysicalPageFrameRange::from_multiboot_entry(mmap_entry).carve(kernelPageFrameRange);
        m_physicalPageFrameRange.push_back(first);
        if(second)
          m_physicalPageFrameRange.push_back(*second);
      }
    }
  }

  template<template<typename> typename Container>
  std::optional<PhysicalPageFrameRange> PhysicalPageFrameAllocator<Container>::allocate(size_t count)
  {
    for(PhysicalPageFrameRange& physicalPageFrameRange : m_physicalPageFrameRange)
    {
      if(physicalPageFrameRange.count>=count)
      {
        auto allocatedPhysicalPageFrameRange = PhysicalPageFrameRange(physicalPageFrameRange.index, count);
        physicalPageFrameRange.index += count;
        physicalPageFrameRange.count -= count;
        return allocatedPhysicalPageFrameRange;
      }
    }
    return std::nullopt;
  }

  template<template<typename> typename Container>
  void PhysicalPageFrameAllocator<Container>::deallocate(PhysicalPageFrameRange freedPhysicalPageFrameRange)
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

    auto try_merge = [&](size_t i) -> void {
      for(size_t j=0; j<i; ++j)
        if(!merge_to(m_physicalPageFrameRange[i], m_physicalPageFrameRange[j]))
          return;
    };

    // Attempt to merge
    auto try_merge_or_insert = [&](PhysicalPageFrameRange& physicalPageFrameRangeToMerge, auto& try_merge_or_insert_ref) -> void{
      for(size_t i=0; i<m_physicalPageFrameRange.size(); ++i)
      {
        auto& physicalPageFrameRange = m_physicalPageFrameRange[i];
        if(merge_to(physicalPageFrameRange, physicalPageFrameRangeToMerge))
        {
          try_merge(i);
          return;
        }

        if(freedPhysicalPageFrameRange < physicalPageFrameRange)
        {
          m_physicalPageFrameRange.insert(&physicalPageFrameRange, physicalPageFrameRangeToMerge); // Insert Before
          return;
        }
      }
      return;
    };

    try_merge_or_insert(freedPhysicalPageFrameRange, try_merge_or_insert);
  }
}

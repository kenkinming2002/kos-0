#include <generic/core/memory/VirtualMemoryRegionAllocator.hpp>

#include <algorithm>
#include <tuple>
#include <generic/utils/Utilities.hpp>

#include <i686/boot/lower_half.hpp>
#include <generic/io/Print.hpp>

#include <generic/core/memory/LinkedListMemoryRegionAllocator.hpp>

namespace core::memory
{
  template<typename Impl>
  VirtualMemoryRegionAllocator<Impl>::VirtualMemoryRegionAllocator(std::byte* begin, std::byte* end)
  {
    this->m_memoryRegions.push_front(*(new MemoryRegion(begin, end)));
  }

  template class VirtualMemoryRegionAllocator<LinkedListMemoryRegionAllocator>;
}


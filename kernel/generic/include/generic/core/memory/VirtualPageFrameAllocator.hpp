#pragma once

#include <generic/core/memory/MemoryRegion.hpp>

#include <optional>

namespace core::memory
{
  class VirtualPageFrameAllocator
  {
  public:
    // NOTE: There is no need for a destructor. All memory is reclaimed at
    //       computer shutdown.
    VirtualPageFrameAllocator(std::byte* begin, std::byte* end);

    VirtualPageFrameAllocator(const VirtualPageFrameAllocator&) = delete;
    VirtualPageFrameAllocator(VirtualPageFrameAllocator&&) = delete;

  public:
    std::optional<MemoryRegion> allocate(size_t count = 1u);
    void deallocate(MemoryRegion freedMemoryRegion);

  private:
    boost::intrusive::slist<MemoryRegion> m_memoryRegions;
  };
}



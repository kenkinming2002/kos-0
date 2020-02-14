#pragma once

#include <i686/core/Paging.hpp>
#include <generic/core/memory/PageFrame.hpp>

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
    std::optional<VirtualPageFrameRange> allocate(size_t count = 1u);
    void deallocate(VirtualPageFrameRange freedVirtualPageFrameRange);

  public:
    enum class MapResult
    {
      SUCCESS,
      ERR_NO_PAGE_TABLE = -1,
      ERR_INVALID_PAGE_TABLE = -2
    };

    MapResult map(PhysicalPageFrameRange physicalPageFrameRange, VirtualPageFrameRange virtualPageFrameRange, 
        std::optional<VirtualPageFrameRange> pageTablePhysicalMemory = std::nullopt) const;
    std::optional<PhysicalPageFrameRange> unmap(VirtualPageFrameRange virtualPageFrameRange) const;

  private:
    std::byte* doFractalMapping(uintptr_t physicalAddress) const;

  private:
    boost::intrusive::slist<VirtualPageFrameRange> m_virtualPageFrameRanges;
  };
}



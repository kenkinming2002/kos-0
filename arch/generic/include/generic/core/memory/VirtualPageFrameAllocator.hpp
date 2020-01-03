#pragma once

#include <i686/core/Paging.hpp>
#include <core/memory/PageFrame.hpp>

#include <optional>

namespace core::memory
{
  class VirtualPageFrameAllocator
  {
  public:
    /**
     * get usable Virtual Memory Chunk
     *
     * @param n  Size of Virtual Memory Chunk to allocated in number of page
     *
     * @return Virtual Memory Chunk allocated
     *
     * @note Virtual Memory Chunk are not claimed until it is mapped and MAY be
     *       returned again on the next call to getUsableVirtualPageFrameRange()
     */
    std::optional<VirtualPageFrameRange> getUsableVirtualPageFrameRange(size_t n) const;

  public:
    enum class MapResult
    {
      SUCCESS,
      ERR_NO_PAGE_TABLE = -1,
      ERR_INVALID_PAGE_TABLE = -2
    };

    MapResult map(PhysicalPageFrameRange physicalPageFrameRange, VirtualPageFrameRange virtualPageFrameRange, 
        std::optional<VirtualPageFrameRange> pageTablePhysicalMemory = std::nullopt) const;
    void unmap(VirtualPageFrameRange virtualPageFrameRange) const;
  };
}



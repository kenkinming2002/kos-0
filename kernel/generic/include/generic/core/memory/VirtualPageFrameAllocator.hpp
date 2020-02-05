#pragma once

#include <i686/core/Paging.hpp>
#include <core/memory/PageFrame.hpp>

#include <optional>

#include <containers/List.hpp>
#include <allocators/CAllocator.hpp>

namespace core::memory
{
  class VirtualPageFrameAllocator
  {
  public:
    VirtualPageFrameAllocator(std::byte* begin, std::byte* end);
    ~VirtualPageFrameAllocator();

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
    //std::optional<VirtualPageFrameRange> getUsableVirtualPageFrameRange(size_t n);

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
    containers::ForwardList<VirtualPageFrameRange, allocators::CAllocator<VirtualPageFrameRange>> m_virtualPageFrameRanges;
  };
}



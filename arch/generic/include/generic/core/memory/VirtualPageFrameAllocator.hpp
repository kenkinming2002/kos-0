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

#if 0
namespace core::memory
{
  struct VirtualFrame
  {
    uint16_t pageDirectoryIndex; /// 0...1023
    uint16_t pageTableIndex;     /// 0...1023
    uint16_t count;

    constexpr static size_t PAGE_DIRECTORY_COUNT = 1024;
    constexpr static size_t PAGE_TABLE_COUNT     = 1024;
    constexpr static size_t PAGE_SIZE            = PageFrame::SIZE;

    constexpr VirtualFrame(uint16_t pageDirectoryIndex, uint16_t pageTableIndex, uint16_t count)
      : pageDirectoryIndex(pageDirectoryIndex), pageTableIndex(pageTableIndex), count(count) {}

    constexpr VirtualFrame(PageFrame* pageFrame, uint16_t count)
      : pageDirectoryIndex((reinterpret_cast<uintptr_t>(pageFrame) / PAGE_SIZE) / PAGE_TABLE_COUNT),
        pageTableIndex    ((reinterpret_cast<uintptr_t>(pageFrame) / PAGE_SIZE) % PAGE_TABLE_COUNT),
        count(count) {}

    constexpr PageFrame* pageFrame() const
    {
      return &static_cast<PageFrame*>(nullptr)[pageDirectoryIndex * PAGE_TABLE_COUNT + pageTableIndex];
    }

  };


  class VirtualFrameAllocator
  {
  public:
    utils::Optional<VirtualFrame> allocate(uint16_t count) const;

  //public:
  //  bool hasPageTable(VirtualFrame virtualFrame) const;
  //  void createPageTable(VirtualFrame virtualFrame, PhysicalFrame memory) const;

  public:
    enum class MapResult
    {
      SUCCESS,
      ERR_NO_PAGE_TABLE = -1,
      ERR_INVALID_PAGE_TABLE = -2
    };

    /** map
     *  
     *  @param virtualFrame     Virtual address to map physical frame to
     *  @param physicalFrame    Physical frame to be mapped to virtual frame
     *  @param pageTableMemory  M
     *
     *  @return value from MapResult
     */
    MapResult map(VirtualFrame virtualFrame, PhysicalFrame physicalFrame, utils::Optional<PhysicalFrame&> pageTableMemory) const;
    PhysicalFrame unmap(VirtualFrame virtualFrame) const;

  public:
    /** reservedFrame
     *  
     *  @return Page Frame usable to temporarily map in data
     */
    constexpr VirtualFrame reservedFrame() const { return VirtualFrame(0, 1023, 1); }
  };
}
#endif




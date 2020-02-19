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
    std::optional<MemoryRegion> allocate(size_t count = 1u);
    void deallocate(MemoryRegion freedMemoryRegion);

  public:
    enum class MapResult
    {
      SUCCESS,
      ERR_NO_PAGE_TABLE = -1,
      ERR_INVALID_PAGE_TABLE = -2
    };

    MapResult map(MemoryRegion physicalMemoryRegion, MemoryRegion virtualMemoryRegion, void* pageTablePhysicalMemory = nullptr) const;
    /*
     * Unmap a virtual memory region
     *
     * @return physical memory region previously mapped to virtualMemoryRegion
     */
    MemoryRegion unmap(MemoryRegion virtualMemoryRegion) const;

  private:
    std::byte* doFractalMapping(uintptr_t physicalAddress) const;

  private:
    boost::intrusive::slist<MemoryRegion> m_memoryRegions;
  };
}



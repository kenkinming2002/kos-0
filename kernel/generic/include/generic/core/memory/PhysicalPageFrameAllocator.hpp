#pragma once

#include <stdint.h>
#include <stddef.h>

#include <optional>

#include <i686/boot/boot.hpp>
#include <generic/core/memory/MemoryRegion.hpp>

namespace core::memory
{
  class PhysicalPageFrameAllocator
  {
  public:
    PhysicalPageFrameAllocator(BootInformation::MemoryMapEntry* memoryMapEntries, size_t length);

    PhysicalPageFrameAllocator(const PhysicalPageFrameAllocator&) = delete;
    PhysicalPageFrameAllocator(PhysicalPageFrameAllocator&&) = delete;

  public:
    std::optional<MemoryRegion> allocate(size_t count = 1u);
    void deallocate(MemoryRegion physicalMemoryRegion);

  private:
    boost::intrusive::slist<MemoryRegion> m_memoryRegions;
  };
}


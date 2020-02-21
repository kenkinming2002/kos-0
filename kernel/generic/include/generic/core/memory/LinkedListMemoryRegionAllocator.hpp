#pragma once

#include <stdint.h>
#include <stddef.h>

#include <optional>

#include <i686/boot/boot.hpp>
#include <generic/core/memory/MemoryRegion.hpp>

namespace core::memory
{
  class LinkedListMemoryRegionAllocator
  {
  public:
    LinkedListMemoryRegionAllocator() = default;
    LinkedListMemoryRegionAllocator(const LinkedListMemoryRegionAllocator&) = delete;
    LinkedListMemoryRegionAllocator(LinkedListMemoryRegionAllocator&&) = delete;

  public:
    std::optional<MemoryRegion> allocate(size_t count = 1u);
    void deallocate(MemoryRegion physicalMemoryRegion);

  protected:
    boost::intrusive::slist<MemoryRegion> m_memoryRegions;
  };
}


#pragma once

#include <generic/core/memory/PhysicalMemoryRegionAllocator.hpp>
#include <generic/core/memory/VirtualMemoryRegionAllocator.hpp>
#include <generic/core/memory/LinkedListMemoryRegionAllocator.hpp>

namespace core::memory
{
  class PageFrameAllocator
  {
  public:
    PageFrameAllocator(PhysicalMemoryRegionAllocator<LinkedListMemoryRegionAllocator>& physicalMemoryRegionAllocator, VirtualMemoryRegionAllocator<LinkedListMemoryRegionAllocator>& virtualMemoryRegionAllocator);

  public:
    std::pair<void*, phyaddr_t> allocate(size_t n);
    void deallocate(void* pageFrames, size_t n);

  private:
    PhysicalMemoryRegionAllocator<LinkedListMemoryRegionAllocator>& m_physicalMemoryRegionAllocator;
    VirtualMemoryRegionAllocator<LinkedListMemoryRegionAllocator>&  m_virtualMemoryRegionAllocator;
  };
}

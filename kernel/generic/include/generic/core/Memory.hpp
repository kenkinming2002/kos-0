#pragma once

#include <generic/core/memory/PageFrameAllocator.hpp>

namespace core
{
  class Memory
  {
  public:
    Memory();

  public:
    void* allocate(size_t n);
    void deallocate(void* pages, size_t n);

  public:
    memory::PhysicalMemoryRegionAllocator<memory::LinkedListMemoryRegionAllocator> m_physicalMemoryRegionAllocator;
    memory::VirtualMemoryRegionAllocator<memory::LinkedListMemoryRegionAllocator> m_virtualMemoryRegionAllocator;

    memory::PageFrameAllocator m_pageFrameAllocator;
  };

  extern Memory gMemory;
}

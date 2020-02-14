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
    core::memory::PhysicalPageFrameAllocator m_physicalPageFrameAllocator;
    core::memory::VirtualPageFrameAllocator m_virtualPageFrameAllocator;

    core::memory::PageFrameAllocator m_pageFrameAllocator;
  };

  extern Memory gMemory;
}

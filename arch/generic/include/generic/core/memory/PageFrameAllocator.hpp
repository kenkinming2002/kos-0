#pragma once

#include <generic/core/memory/PhysicalPageFrameAllocator.hpp>
#include <generic/core/memory/VirtualPageFrameAllocator.hpp>

namespace core::memory
{

  template<typename PhysicalPageFrameAllocator, typename VirtualPageFrameAllocator>
  class PageFrameAllocator
  {
  public:
    PageFrameAllocator(PhysicalPageFrameAllocator&& physicalPageFrameAllocator, VirtualPageFrameAllocator&& virtualPageFrameAllocator);

  public:
    PageFrame<>* allocate(size_t n);
    void deallocate(PageFrame<>* pageFrames, size_t n);

  private:
    PhysicalPageFrameAllocator m_physicalPageFrameAllocator;
    VirtualPageFrameAllocator  m_virtualPageFrameAllocator;
  };
}

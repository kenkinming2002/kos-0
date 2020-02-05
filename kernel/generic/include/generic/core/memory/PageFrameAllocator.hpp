#pragma once

#include <generic/core/memory/PhysicalPageFrameAllocator.hpp>
#include <generic/core/memory/VirtualPageFrameAllocator.hpp>
#include <generic/core/memory/PageFrame.hpp>

namespace core::memory
{
  struct MemoryMapping
  {
  public:
    PageFrame<>* virtualPageFrame, physicalPageFrame;
    size_t count;
  };

  class PageFrameAllocator
  {
  public:
    PageFrameAllocator(PhysicalPageFrameAllocator& physicalPageFrameAllocator, VirtualPageFrameAllocator& virtualPageFrameAllocator);

  public:
    PageFrame<>* allocate(size_t n);
    void deallocate(PageFrame<>* pageFrames, size_t n);

  private:
    PhysicalPageFrameAllocator& m_physicalPageFrameAllocator;
    VirtualPageFrameAllocator&  m_virtualPageFrameAllocator;
  };
}

#include <generic/core/memory/PageFrameAllocator.hpp>

#include <io/Framebuffer.hpp>

#include <utility>

namespace core::memory
{
  PageFrameAllocator::PageFrameAllocator(
      PhysicalPageFrameAllocator& physicalPageFrameAllocator, VirtualPageFrameAllocator& virtualPageFrameAllocator)
  : m_physicalPageFrameAllocator(physicalPageFrameAllocator), 
    m_virtualPageFrameAllocator(virtualPageFrameAllocator)
  {}

  PageFrame<>* PageFrameAllocator::allocate(size_t n)
  {
    io::print(__PRETTY_FUNCTION__);

    std::optional<VirtualPageFrameRange> virtualPageFrameRange  = m_virtualPageFrameAllocator.allocate(n);
    if(!virtualPageFrameRange)
      return nullptr;

    auto physicalPageFrameRange = m_physicalPageFrameAllocator.allocate(n);
    if(!physicalPageFrameRange)
      return nullptr;

    switch(m_virtualPageFrameAllocator.map(*physicalPageFrameRange, *virtualPageFrameRange))
    {
      case core::memory::VirtualPageFrameAllocator::MapResult::ERR_NO_PAGE_TABLE:
      {
        auto pageTablePhysicalMemory = m_physicalPageFrameAllocator.allocate(1);
        if(!pageTablePhysicalMemory)
        {
          io::print("Failed to Allocate Memory for Page Table");
          io::print("-----FUNCTION EXIT-----");

          // Cleanup
          m_physicalPageFrameAllocator.deallocate(*physicalPageFrameRange);
          m_virtualPageFrameAllocator.deallocate(*virtualPageFrameRange);

          return nullptr;
        }
        if(m_virtualPageFrameAllocator.map(*physicalPageFrameRange, *virtualPageFrameRange, *pageTablePhysicalMemory) != 
            core::memory::VirtualPageFrameAllocator::MapResult::SUCCESS)
        {
          io::print("Failed to map");
          io::print("-----FUNCTION EXIT-----");

          // Cleanup
          m_physicalPageFrameAllocator.deallocate(*physicalPageFrameRange);
          m_virtualPageFrameAllocator.deallocate(*virtualPageFrameRange);
          m_physicalPageFrameAllocator.deallocate(*pageTablePhysicalMemory);

          return nullptr; 
        }
        break;
      }
      case core::memory::VirtualPageFrameAllocator::MapResult::ERR_INVALID_PAGE_TABLE:
      {
        io::print("ERR_INVALID_PAGE_TABLE");
        io::print("-----FUNCTION EXIT-----");

        // Cleanup
        m_physicalPageFrameAllocator.deallocate(*physicalPageFrameRange);
        m_virtualPageFrameAllocator.deallocate(*virtualPageFrameRange);

        return nullptr;
      }
      case core::memory::VirtualPageFrameAllocator::MapResult::SUCCESS:
        break;
    }
    io::print("PageFrames: ", reinterpret_cast<uintptr_t>(virtualPageFrameRange->toPageFrames()));
    io::print("-----FUNCTION EXIT-----");
    return virtualPageFrameRange->toPageFrames();
  }

  void PageFrameAllocator::deallocate(PageFrame<>* pageFrames, size_t n)
  {
    auto virtualPageFrameRange = VirtualPageFrameRange::fromPageFrames(pageFrames, n);
    auto physicalPageFrameRange = m_virtualPageFrameAllocator.unmap(virtualPageFrameRange);

    m_virtualPageFrameAllocator.deallocate(virtualPageFrameRange);
    if(physicalPageFrameRange)
      m_physicalPageFrameAllocator.deallocate(*physicalPageFrameRange);
    else
    {
      // THIS is not normal but we can still keep on running
    }
  }
}

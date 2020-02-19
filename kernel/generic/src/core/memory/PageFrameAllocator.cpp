#include <generic/core/memory/PageFrameAllocator.hpp>

#include <generic/io/Print.hpp>

#include <utility>

namespace core::memory
{
  PageFrameAllocator::PageFrameAllocator(
      PhysicalPageFrameAllocator& physicalPageFrameAllocator, VirtualPageFrameAllocator& virtualPageFrameAllocator)
  : m_physicalPageFrameAllocator(physicalPageFrameAllocator), 
    m_virtualPageFrameAllocator(virtualPageFrameAllocator)
  {}

  void* PageFrameAllocator::allocate(size_t n)
  {
    auto virtualMemoryRegion  = m_virtualPageFrameAllocator.allocate(n);
    if(!virtualMemoryRegion)
      return nullptr;

    auto physicalMemoryRegion = m_physicalPageFrameAllocator.allocate(n);
    if(!physicalMemoryRegion)
      return nullptr;

    switch(m_virtualPageFrameAllocator.map(*physicalMemoryRegion, *virtualMemoryRegion))
    {
      case core::memory::VirtualPageFrameAllocator::MapResult::ERR_NO_PAGE_TABLE:
      {
        auto pageTablePhysicalMemory = m_physicalPageFrameAllocator.allocate(1);
        if(!pageTablePhysicalMemory)
        {
          // Cleanup
          m_physicalPageFrameAllocator.deallocate(*physicalMemoryRegion);
          m_virtualPageFrameAllocator.deallocate(*virtualMemoryRegion);

          return nullptr;
        }
        if(m_virtualPageFrameAllocator.map(*physicalMemoryRegion, *virtualMemoryRegion, reinterpret_cast<void*>(pageTablePhysicalMemory->begin())) != 
            core::memory::VirtualPageFrameAllocator::MapResult::SUCCESS)
        {
          // Cleanup
          m_physicalPageFrameAllocator.deallocate(*physicalMemoryRegion);
          m_virtualPageFrameAllocator.deallocate(*virtualMemoryRegion);
          m_physicalPageFrameAllocator.deallocate(*pageTablePhysicalMemory);

          return nullptr; 
        }
        break;
      }
      case core::memory::VirtualPageFrameAllocator::MapResult::ERR_INVALID_PAGE_TABLE:
      {
        // Cleanup
        m_physicalPageFrameAllocator.deallocate(*physicalMemoryRegion);
        m_virtualPageFrameAllocator.deallocate(*virtualMemoryRegion);

        return nullptr;
      }
      case core::memory::VirtualPageFrameAllocator::MapResult::SUCCESS:
        break;
    }
    return reinterpret_cast<void*>(virtualMemoryRegion->begin());
  }

  void PageFrameAllocator::deallocate(void* pageFrames, size_t n)
  {
    auto virtualMemoryRegion = MemoryRegion(pageFrames, n * PAGE_SIZE);
    auto physicalMemoryRegion = m_virtualPageFrameAllocator.unmap(virtualMemoryRegion);

    m_virtualPageFrameAllocator.deallocate(virtualMemoryRegion);
    m_physicalPageFrameAllocator.deallocate(physicalMemoryRegion);
  }
}

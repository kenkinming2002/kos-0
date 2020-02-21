#include <generic/core/memory/PageFrameAllocator.hpp>

#include <i686/core/memory/Paging.hpp>

#include <generic/io/Print.hpp>

#include <utility>

namespace core::memory
{
  PageFrameAllocator::PageFrameAllocator(
      PhysicalMemoryRegionAllocator<LinkedListMemoryRegionAllocator>& physicalMemoryRegionAllocator, VirtualMemoryRegionAllocator<LinkedListMemoryRegionAllocator>& virtualMemoryRegionAllocator)
  : m_physicalMemoryRegionAllocator(physicalMemoryRegionAllocator), 
    m_virtualMemoryRegionAllocator(virtualMemoryRegionAllocator)
  {}

  void* PageFrameAllocator::allocate(size_t n)
  {
    auto& memoryMapping = utils::deref_cast<core::memory::MemoryMapping>(kernelMemoryMapping);

    auto virtualMemoryRegion  = m_virtualMemoryRegionAllocator.allocate(n);
    if(!virtualMemoryRegion)
      return nullptr;

    auto physicalMemoryRegion = m_physicalMemoryRegionAllocator.allocate(n);
    if(!physicalMemoryRegion)
      return nullptr;

    switch(memoryMapping.map(*physicalMemoryRegion, *virtualMemoryRegion))
    {
      case core::memory::MemoryMapping::MapResult::ERR_NO_PAGE_TABLE:
      {
        auto pageTablePhysicalMemory = m_physicalMemoryRegionAllocator.allocate(1);
        if(!pageTablePhysicalMemory)
        {
          // Cleanup
          m_physicalMemoryRegionAllocator.deallocate(*physicalMemoryRegion);
          m_virtualMemoryRegionAllocator.deallocate(*virtualMemoryRegion);

          return nullptr;
        }
        if(memoryMapping.map(*physicalMemoryRegion, *virtualMemoryRegion, reinterpret_cast<void*>(pageTablePhysicalMemory->begin())) != 
            core::memory::MemoryMapping::MapResult::SUCCESS)
        {
          // Cleanup
          m_physicalMemoryRegionAllocator.deallocate(*physicalMemoryRegion);
          m_virtualMemoryRegionAllocator.deallocate(*virtualMemoryRegion);
          m_physicalMemoryRegionAllocator.deallocate(*pageTablePhysicalMemory);

          return nullptr; 
        }
        break;
      }
      case core::memory::MemoryMapping::MapResult::ERR_INVALID_PAGE_TABLE:
      {
        // Cleanup
        m_physicalMemoryRegionAllocator.deallocate(*physicalMemoryRegion);
        m_virtualMemoryRegionAllocator.deallocate(*virtualMemoryRegion);

        return nullptr;
      }
      case core::memory::MemoryMapping::MapResult::SUCCESS:
        break;
    }
    return reinterpret_cast<void*>(virtualMemoryRegion->begin());
  }

  void PageFrameAllocator::deallocate(void* pageFrames, size_t n)
  {
    auto& memoryMapping = utils::deref_cast<core::memory::MemoryMapping>(kernelMemoryMapping);

    auto virtualMemoryRegion = MemoryRegion(pageFrames, n * PAGE_SIZE);
    auto physicalMemoryRegion = memoryMapping.unmap(virtualMemoryRegion);

    m_virtualMemoryRegionAllocator.deallocate(virtualMemoryRegion);
    m_physicalMemoryRegionAllocator.deallocate(physicalMemoryRegion);
  }
}

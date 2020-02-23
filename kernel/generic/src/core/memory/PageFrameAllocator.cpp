#include <generic/core/memory/PageFrameAllocator.hpp>

#include <i686/core/memory/MemoryMapping.hpp>

#include <generic/io/Print.hpp>

#include <utility>

namespace core::memory
{
  PageFrameAllocator::PageFrameAllocator(
      PhysicalMemoryRegionAllocator<LinkedListMemoryRegionAllocator>& physicalMemoryRegionAllocator, VirtualMemoryRegionAllocator<LinkedListMemoryRegionAllocator>& virtualMemoryRegionAllocator)
  : m_physicalMemoryRegionAllocator(physicalMemoryRegionAllocator), 
    m_virtualMemoryRegionAllocator(virtualMemoryRegionAllocator)
  {}

  std::pair<void*, phyaddr_t> PageFrameAllocator::allocate(size_t n)
  {
    auto& memoryMapping = currentMemoryMapping;

    auto virtualMemoryRegion  = m_virtualMemoryRegionAllocator.allocate(n);
    if(!virtualMemoryRegion)
      return {nullptr, 0u};

    auto physicalMemoryRegion = m_physicalMemoryRegionAllocator.allocate(n);
    if(!physicalMemoryRegion)
      return {nullptr, 0u};

    memoryMapping.map(*physicalMemoryRegion, *virtualMemoryRegion, Access::SUPERVISOR_ONLY, Permission::READ_WRITE);
    return {reinterpret_cast<void*>(virtualMemoryRegion->begin()), physicalMemoryRegion->begin()};
  }

  void PageFrameAllocator::deallocate(void* pageFrames, size_t n)
  {
    auto& memoryMapping = currentMemoryMapping;

    auto virtualMemoryRegion = MemoryRegion(pageFrames, n * PAGE_SIZE);
    auto physicalMemoryRegion = memoryMapping.unmap(virtualMemoryRegion);

    m_virtualMemoryRegionAllocator.deallocate(virtualMemoryRegion);
    m_physicalMemoryRegionAllocator.deallocate(physicalMemoryRegion);
  }
}

#include <generic/core/memory/PageFrameAllocator.hpp>

#include <io/Framebuffer.hpp>

#include <utility>

namespace core::memory
{
  template<typename PhysicalPageFrameAllocator, typename VirtualPageFrameAllocator>
  PageFrameAllocator<PhysicalPageFrameAllocator, VirtualPageFrameAllocator>::PageFrameAllocator(
      PhysicalPageFrameAllocator&& physicalPageFrameAllocator, VirtualPageFrameAllocator&& virtualPageFrameAllocator)
  : m_physicalPageFrameAllocator(std::move(physicalPageFrameAllocator)), 
    m_virtualPageFrameAllocator(std::move(virtualPageFrameAllocator))
  {}

  template<typename PhysicalFrameAllocator, typename VirtualFrameAllocator>
  PageFrame<>* PageFrameAllocator<PhysicalFrameAllocator, VirtualFrameAllocator>::allocate(size_t n)
  {
    std::optional<VirtualPageFrameRange> virtualPageFrameRange  = m_virtualPageFrameAllocator.getUsableVirtualPageFrameRange(n);
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
          return nullptr; // TODO: Cleanup
        if(m_virtualPageFrameAllocator.map(*physicalPageFrameRange, *virtualPageFrameRange, *pageTablePhysicalMemory) != 
            core::memory::VirtualPageFrameAllocator::MapResult::SUCCESS)
          return nullptr; // TODO: Cleanup
        break;
      }
      case core::memory::VirtualPageFrameAllocator::MapResult::ERR_INVALID_PAGE_TABLE:
        return nullptr;
      case core::memory::VirtualPageFrameAllocator::MapResult::SUCCESS:
        break;
    }
    return virtualPageFrameRange->toPageFrames();
  }

  template class PageFrameAllocator<StaticPhysicalPageFrameAllocator, VirtualPageFrameAllocator>;

  // Explcit Instantiation
  //template class PageFrameAllocator<BootPhysicalFrameAllocator, VirtualFrameAllocator>;
  //template class PageFrameAllocator<PhysicalFrameAllocator, VirtualFrameAllocator>;
}

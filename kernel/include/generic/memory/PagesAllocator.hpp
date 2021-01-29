#pragma once

#include <generic/BootInformation.hpp>
#include <generic/memory/LinkedListPagesAllocator.hpp>

#include <i686/memory/MemoryMapping.hpp>

namespace core::memory
{
  class PagesAllocator
  {
  public:
    PagesAllocator()
    {
      for(size_t i=0; i<bootInformation->memoryMapEntriesCount; ++i)
      {
        const auto& memoryMapEntry = bootInformation->memoryMapEntries[i];
        if(memoryMapEntry.type == MemoryMapEntry::Type::AVAILABLE)
          m_physicalPagesAllocator.markAsAvailable(Pages::fromConservative(memoryMapEntry.addr, memoryMapEntry.len));
      }

      for(size_t i=0; i<bootInformation->moduleEntriesCount; ++i)
      {
        const auto& moduleEntry = bootInformation->moduleEntries[i];
        m_physicalPagesAllocator.markAsUsed(Pages::fromAggressive(moduleEntry.addr, moduleEntry.len));
      }

      for(size_t i=0; i<bootInformation->memoryRegionsCount; ++i)
      {
        const auto& memoryRegions = bootInformation->memoryRegions[i];
        m_physicalPagesAllocator.markAsUsed(Pages::fromAggressive(memoryRegions.addr, memoryRegions.len));
      }

      uintptr_t kernel_heap_begin = 0xD0000000;
      uintptr_t kernel_heap_end   = 0xE0000000;
      m_virtualPagesAllocator.markAsAvailable(Pages::from(kernel_heap_begin, kernel_heap_end-kernel_heap_begin));

      for(const auto& pages : m_physicalPagesAllocator.list())
        io::printf("Physical memory from 0x%lx to 0x%lx with length 0x%lx\n", pages.address(), pages.address()+pages.length(), pages.length());
    }

  public:
    std::optional<Pages> allocPhysicalPages(size_t count) { return m_physicalPagesAllocator.allocate(count); }
    void freePhysicalPages(Pages pages) { m_physicalPagesAllocator.deallocate(pages); }

    std::optional<Pages> allocVirtualPages(size_t count) { return m_virtualPagesAllocator.allocate(count); }
    void freeVirtualPages(Pages pages) { m_virtualPagesAllocator.deallocate(pages); }

    std::optional<Pages> allocMappedPages(size_t count)
    {
      auto virtualPages = allocVirtualPages(count);
      if(!virtualPages)
        return std::nullopt;

      MemoryMapping::current().map(*virtualPages, common::memory::Access::SUPERVISOR_ONLY, common::memory::Permission::READ_WRITE);
      return virtualPages;
    }

    void freeMappedPages(Pages pages)
    {
      MemoryMapping::current().unmap(pages);
      freeVirtualPages(pages);
    }

  private:

    /* TODO: Use multiple physical pages allocator for DMA and non-DMA memory and
     *       so on */
    LinkedListPagesAllocator m_physicalPagesAllocator;
    LinkedListPagesAllocator m_virtualPagesAllocator;
  };

}

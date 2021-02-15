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
      uintptr_t kernel_heap_begin = 0xD0000000;
      uintptr_t kernel_heap_end   = 0xE0000000;
      m_virtualPagesAllocator.markAsAvailable(Pages::from(kernel_heap_begin, kernel_heap_end-kernel_heap_begin));
    }

  public:
    rt::Optional<Pages> allocVirtualPages(size_t count) { return m_virtualPagesAllocator.allocate(count); }
    void freeVirtualPages(Pages pages) { m_virtualPagesAllocator.deallocate(pages); }

    rt::Optional<Pages> allocMappedPages(size_t count)
    {
      auto virtualPages = allocVirtualPages(count);
      if(!virtualPages)
        return rt::nullOptional;

      MemoryMapping::current().map(*virtualPages, common::memory::Access::SUPERVISOR_ONLY, common::memory::Permission::READ_WRITE);
      return virtualPages;
    }

    void freeMappedPages(Pages pages)
    {
      MemoryMapping::current().unmap(pages);
      freeVirtualPages(pages);
    }

  private:
    LinkedListPagesAllocator m_virtualPagesAllocator;
  };

}

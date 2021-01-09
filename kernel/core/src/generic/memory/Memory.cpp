#include <core/generic/memory/Memory.hpp>

#include <core/generic/Init.hpp>
#include <core/generic/Panic.hpp>
#include <core/generic/io/Print.hpp>
#include <core/generic/memory/LinkedListPagesAllocator.hpp>

#include <core/i686/memory/MemoryMapping.hpp>

#include <boot/i686/BootInformation.hpp>

#include <liballoc_1_1.h>

#include <optional>

#include <assert.h>

namespace core::memory
{
  /*
   * We are using preallocated buffer for boot time pages allocation now.
   *
   * Alternatively, we can search for physical memory directly from
   * BOOT_INFORMATION and map it in ourselves(Currently, we do not have to do
   * the mapping ourselves here since early_heap in the .bss section and will
   * be mapped in for at boot time). This would be useful, if we were to use a
   * BuddyAllocator in the future due to the its larger initial memory
   * consumption and the possibility that early_heap, which is statically
   * allocated, is too small. However, it should be fine for
   * LinkedListPagesAllocator, which has minimal memory consumption.
   *
   * The reason we use the size of 16 pages is due to a config option in
   * liballoc - l_pageCount - set to 16, which implies that a minimum of 16
   * pages would be requested each time in advance even if only a few bytes
   * were to be allocated as in the case of LinkedListPagesAllocator for
   * book-keeping purposes. That is not to say that it is not possible for
   * count to exceed 16 or for liballoc_alloc to be called multiple time
   * before our real allocator is initialized, but that should not happen in
   * the case of LinkedListPagesAllocator with low memory overhead.
   */
  extern "C" char kernel_physical_start[];
  extern "C" char kernel_physical_end[];
  extern "C" char kernel_virtual_start[];
  extern "C" char kernel_virtual_end[];

  bool initialized = false;

  class BootPagesAllocator
  {
  public:
    constexpr BootPagesAllocator() = default;

    std::optional<Pages> allocMappedPages(size_t count)
    {
      assert(!m_used && "Early Heap already used but the memory system is not yet initialized");
      assert(count == 16 && "Early heap size mismatch");

      return Pages::from(reinterpret_cast<uintptr_t>(m_earlyHeap), sizeof m_earlyHeap);
    }


  private:
    bool m_used = false;
    alignas(PAGE_SIZE) char m_earlyHeap[PAGE_SIZE * 16] = {};
  };

  class PagesAllocator
  {
  public:
    PagesAllocator()
    {
      for(size_t i=0; i<BOOT_INFORMATION.memoryMapEntriesCount; ++i)
      {
        const auto& memoryMapEntry = BOOT_INFORMATION.memoryMapEntries[i];
        if(memoryMapEntry.type == MemoryMapEntry::Type::AVAILABLE)
          m_physicalPagesAllocator.markAsAvailable(Pages::fromConservative(memoryMapEntry.addr, memoryMapEntry.len));
      }

      for(size_t i=0; i<BOOT_INFORMATION.moduleEntriesCount; ++i)
      {
        const auto& moduleEntry = BOOT_INFORMATION.moduleEntries[i];
        m_physicalPagesAllocator.markAsUsed(Pages::fromAggressive(moduleEntry.addr, moduleEntry.len));
      }

      m_physicalPagesAllocator.markAsUsed(Pages::fromAggressive(
        reinterpret_cast<uintptr_t>(kernel_physical_start), 
        reinterpret_cast<uintptr_t>(kernel_physical_end)-reinterpret_cast<uintptr_t>(kernel_physical_start)
      ));

      uintptr_t kernel_heap_begin = reinterpret_cast<uintptr_t>(kernel_virtual_end);
      uintptr_t kernel_heap_end   = 0xD0000000;
      m_virtualPagesAllocator.markAsAvailable(Pages::fromConservative(kernel_heap_begin, kernel_heap_end-kernel_heap_begin));

      for(const auto& pages : m_physicalPagesAllocator.list())
        io::printf("Physical memory from 0x%lx to 0x%lx with length 0x%lx\n", pages.address(), pages.address()+pages.length(), pages.length());

      initialized = true;
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

      MemoryMapping::current->map(*virtualPages, common::memory::Access::ALL, common::memory::Permission::READ_WRITE);
      return virtualPages;
    }

    void freeMappedPages(Pages pages)
    {
      MemoryMapping::current->unmap(pages);
      freeVirtualPages(pages);
    }

  private:

    /* TODO: Use multiple physical pages allocator for DMA and non-DMA memory and
     *       so on */
    LinkedListPagesAllocator m_physicalPagesAllocator;
    LinkedListPagesAllocator m_virtualPagesAllocator;
  };

  constinit BootPagesAllocator bootPagesAllocator;
  INIT_EARLY PagesAllocator pagesAllocator;

  std::optional<Pages> allocPhysicalPages(size_t count) { return pagesAllocator.allocPhysicalPages(count); }
  void freePhysicalPages(Pages pages) { pagesAllocator.freePhysicalPages(pages); }

  std::optional<Pages> allocVirtualPages(size_t count) { return pagesAllocator.allocVirtualPages(count); }
  void freeVirtualPages(Pages pages) { pagesAllocator.freeVirtualPages(pages); }

  std::optional<Pages> allocMappedPages(size_t count) { return initialized ? pagesAllocator.allocMappedPages(count) : bootPagesAllocator.allocMappedPages(count); }
  void freeMappedPages(Pages pages) { pagesAllocator.freeMappedPages(pages); }

  void* malloc(size_t size) { return ::kmalloc(size); }
  void* realloc(void* ptr, size_t size) { return ::krealloc(ptr, size); }
  void* calloc(size_t nmemb, size_t size) { return ::kcalloc(nmemb, size); }
  void free(void* ptr) { return ::kfree(ptr); }

}

#include <generic/memory/Memory.hpp>

#include <i686/memory/MemoryMapping.hpp>

#include <liballoc_1_1.h>

namespace core::memory
{
  /*
   * We are using preallocated buffer for boot time pages allocation now.
   *
   * Alternatively, we can search for physical memory directly from
   * bootInformation->and map it in ourselves(Currently, we do not have to do
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

  void initialize()
  {
    MemoryMapping::initialize();
    initializePhysical();
    initializeVirtual();
  }

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

  void* malloc(size_t size) { return ::kmalloc(size); }
  void* realloc(void* ptr, size_t size) { return ::krealloc(ptr, size); }
  void* calloc(size_t nmemb, size_t size) { return ::kcalloc(nmemb, size); }
  void free(void* ptr) { return ::kfree(ptr); }

}

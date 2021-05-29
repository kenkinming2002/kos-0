#include <generic/memory/Memory.hpp>

#include <i686/memory/MemoryMapping.hpp>

#include <liballoc_1_1.h>

namespace core::memory
{
  namespace
  {
    void test()
    {
      rt::log("Testing memory allocation and deallocation...\n");

      for(size_t j=0; j<128;++j)
      {
        rt::logf("Iteration %lu\n", j);
        char* memorys[200] = {};

        for(size_t i=0; i<200; ++i)
        {
          memorys[i] = static_cast<char*>(core::memory::malloc(0x1000));

          /* Note: Don't be stupid like me and try to use break to break out of
           * nested loop and be puzzled as to why the loop does not end */
          if(memorys[i] == nullptr)
            goto end;

          const char* str = "deadbeef";
          for(size_t k=0; k<0x100; ++k)
          {
            static_cast<volatile char*>(memorys[i])[k]=str[k%8];
            ASSERT(static_cast<volatile char*>(memorys[i])[k]==str[k%8]);
          }
        }

        for(size_t i=0; i<200; ++i)
        {
          if(memorys[i] == nullptr)
            break;
          core::memory::free(static_cast<void*>(memorys[i]));
        }
      }
  end:

      rt::log("Done\n");
    }
  }

  void initialize()
  {
    MemoryMapping::initialize();
    initializePhysical();
    initializeVirtual();

    test();
  }

  rt::Optional<Pages> mapPages(Pages physicalPages)
  {
    auto virtualPages = allocVirtualPages(physicalPages.count);
    if(!virtualPages)
      return rt::nullOptional;

    MemoryMapping::current().map(*virtualPages, common::memory::Access::SUPERVISOR_ONLY, common::memory::Permission::READ_WRITE, physicalPages);
    return virtualPages;
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

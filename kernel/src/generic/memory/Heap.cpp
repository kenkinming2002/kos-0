#include <generic/memory/Heap.hpp>

#include <i686/memory/MemoryMapping.hpp>

#include <generic/memory/MemoryRegions.hpp>
#include <generic/memory/Syscalls.hpp>
#include <generic/BootInformation.hpp>

#include <librt/Log.hpp>
#include <librt/Assert.hpp>

#include <generic/SpinLock.hpp>

#include <new>

namespace core::memory
{
  namespace
  {
    rt::containers::StaticVector<MemoryRegion, MAX_MEMORY_REGIONS_COUNT> prepare()
    {
      rt::containers::StaticVector<MemoryRegion, MAX_MEMORY_REGIONS_COUNT> memoryRegions;
      for(size_t i=0; i<bootInformation->memoryMapEntriesCount; ++i)
        if(bootInformation->memoryMapEntries[i].type == MemoryMapEntry::Type::AVAILABLE)
        {
          const auto& memoryMapEntries = bootInformation->memoryMapEntries[i];
          auto memoryRegion = MemoryRegion{physToVirt(memoryMapEntries.addr), memoryMapEntries.len};
          add(memoryRegions, memoryRegion);
        }

      for(size_t i=0; i<bootInformation->moduleEntriesCount; ++i)
      {
        const auto& moduleEntries = bootInformation->moduleEntries[i];
        auto memoryRegion = MemoryRegion{physToVirt(moduleEntries.addr), moduleEntries.len};
        remove(memoryRegions, memoryRegion);
      }

      for(size_t i=0; i<bootInformation->reservedMemoryRegionsCount; ++i)
      {
        const auto& reservedMemoryRegions = bootInformation->reservedMemoryRegions[i];
        auto memoryRegion = MemoryRegion{physToVirt(reservedMemoryRegions.addr), reservedMemoryRegions.len};
        remove(memoryRegions, memoryRegion);
      }

      sanitize(memoryRegions);
      return memoryRegions;
    }

    struct PagesHeader
    {
      PagesHeader* prev;
      PagesHeader* next;
      size_t count;
    };

    PagesHeader* head;
    void initializePagesHeader()
    {
      const auto memoryRegions = prepare();
      for(size_t i=0; i<memoryRegions.size(); ++i)
      {
        const auto& memoryRegion = memoryRegions[i];
        auto& pagesHeader = *reinterpret_cast<PagesHeader*>(memoryRegion.addr);

        if(i == 0)
          head = &pagesHeader;

        rt::logf("begin:%lx, end:%lx\n", memoryRegion.begin(), memoryRegion.end());

        pagesHeader.prev = i   != 0                    ? reinterpret_cast<PagesHeader*>(memoryRegions[i-1].addr) : nullptr;
        pagesHeader.next = i+1 != memoryRegions.size() ? reinterpret_cast<PagesHeader*>(memoryRegions[i+1].addr) : nullptr;
        pagesHeader.count = memoryRegion.length / PAGE_SIZE;
      }
    }

    void test()
    {
      rt::logf("Testing memory allocation and deallocation...\n");

      for(size_t j=0; j<128;++j)
      {
        char* memorys[200] = {};
        for(size_t i=0; i<200; ++i)
        {
          memorys[i] = new char[0x1000];

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

          delete[] memorys[i];
        }
      }
  end:

      rt::logf("Done\n");
    }
  }

  void initializeHeap()
  {
    initializePagesHeader();
    test();
  }

  constinit static core::SpinLock lock;

  void* allocPages(size_t count)
  {
    core::LockGuard guard(lock);
    for(auto* pagesHeader = head; pagesHeader;  pagesHeader = pagesHeader->next)
    {
      if(pagesHeader->count < count)
        continue;

      if(pagesHeader->count > count)
      {
        // We are too large, shrink self
        auto* newPagesHeader = reinterpret_cast<PagesHeader*>(reinterpret_cast<uintptr_t>(pagesHeader) + count * PAGE_SIZE);
        newPagesHeader->prev = pagesHeader->prev;
        newPagesHeader->next = pagesHeader->next;
        newPagesHeader->count = pagesHeader->count - count;

        if(pagesHeader->prev)
          pagesHeader->prev->next = newPagesHeader;

        if(pagesHeader->next)
          pagesHeader->next->prev = newPagesHeader;

        if(head == pagesHeader)
          head = newPagesHeader;
      }
      else
      {
        // Fit perfectly, unlink self
        if(pagesHeader->prev)
          pagesHeader->prev->next = pagesHeader->next;

        if(pagesHeader->next)
          pagesHeader->next->prev = pagesHeader->prev;

        if(head == pagesHeader)
          head = pagesHeader->next;
      }

      return pagesHeader;
    }

    return nullptr;
  }

  void freePages(void* pages, size_t count)
  {
    core::LockGuard guard(lock);
    auto* pagesHeader = static_cast<PagesHeader*>(pages);
    pagesHeader->count = count;
    pagesHeader->prev = nullptr;
    pagesHeader->next = head;

    head->prev = pagesHeader;
    head = pagesHeader;
  }
}

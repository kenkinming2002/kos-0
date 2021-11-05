#include <i686/memory/KMap.hpp>

#include <generic/PerCPU.hpp>

namespace core::memory
{
  namespace
  {
    PageDirectory& getCurrentPageDirectory()
    {
      physaddr_t currentPageDirectoryPhyaddr;
      asm volatile ("mov %[currentPageDirectoryPhyaddr], cr3" : [currentPageDirectoryPhyaddr]"=r"(currentPageDirectoryPhyaddr) : : "memory");
      PageDirectory* currentPageDirectory = reinterpret_cast<PageDirectory*>(physToVirt(currentPageDirectoryPhyaddr));
      return *currentPageDirectory;
    }
  }
  uintptr_t kmap(physaddr_t physaddr)
  {
    size_t pageDirectoryEntryIndex = 1024 - getCpusCount() + cpuidCurrent();

    size_t offset            = physaddr % LARGE_PAGE_SIZE;

    physaddr_t physaddr_base = physaddr / LARGE_PAGE_SIZE * LARGE_PAGE_SIZE;
    uintptr_t addr_base      = pageDirectoryEntryIndex * LARGE_PAGE_SIZE;
    uintptr_t addr           = addr_base + offset;

    auto& pageDirectory = getCurrentPageDirectory();
    auto& lastPageDirectoryEntry = pageDirectory[pageDirectoryEntryIndex];

    ASSERT(!lastPageDirectoryEntry.present());
    lastPageDirectoryEntry = PageDirectoryEntry(physaddr_base, CacheMode::DISABLED, WriteMode::WRITE_BACK, Access::SUPERVISOR_ONLY, Permission::READ_WRITE, PageSize::LARGE);
    ASSERT(lastPageDirectoryEntry.present());

    asm volatile ( "invlpg [%[addr_base]]" : : [addr_base]"r"(addr_base) : "memory");
    return addr;
  }

  void kunmap(uintptr_t addr)
  {
    size_t pageDirectoryEntryIndex = 1024 - getCpusCount() + cpuidCurrent();

    uintptr_t addr_base = addr / LARGE_PAGE_SIZE * LARGE_PAGE_SIZE;

    auto& pageDirectory = getCurrentPageDirectory();
    auto& lastPageDirectoryEntry = pageDirectory[pageDirectoryEntryIndex];

    ASSERT(lastPageDirectoryEntry.present());
    lastPageDirectoryEntry = PageDirectoryEntry();
    ASSERT(!lastPageDirectoryEntry.present());

    asm volatile ( "invlpg [%[addr_base]]" : : [addr_base]"r"(addr_base) : "memory");
  }
}

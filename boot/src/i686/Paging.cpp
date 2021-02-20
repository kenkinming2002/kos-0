#include <boot/i686/Paging.hpp>

#include <librt/Panic.hpp>
#include <boot/generic/Config.h>
#include <boot/generic/Memory.hpp>

#include <common/i686/memory/Paging.hpp>

#include <librt/Log.hpp>
#include <librt/Iterator.hpp>

#include <librt/Assert.hpp>
#include <librt/Strings.hpp>

#include <librt/Algorithm.hpp>

namespace boot::memory
{
  using namespace common::memory;

  PageDirectory* pageDirectory;
  namespace
  {
    uintptr_t nextVirtaddr = 0;
    PageTableEntry& getPageTableEntry(BootInformation& bootInformation, uintptr_t virtaddr)
    {
      size_t pageDirectoryIndex = virtaddr / LARGE_PAGE_SIZE;
      size_t pageTableIndex     = (virtaddr % LARGE_PAGE_SIZE) / PAGE_SIZE;

      auto& pageDirectoryEntry = (*pageDirectory)[pageDirectoryIndex];
      if(!pageDirectoryEntry.present())
      {
        PageTable* pageTable = static_cast<PageTable*>(allocPages(bootInformation, 1, ReservedMemoryRegion::Type::PAGING));
        pageDirectoryEntry = PageDirectoryEntry(reinterpret_cast<uintptr_t>(pageTable), CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::ALL, Permission::READ_WRITE);
      }

      auto pageTable = reinterpret_cast<PageTable*>(pageDirectoryEntry.address());
      auto& pageTableEntry = (*pageTable)[pageTableIndex];
      return pageTableEntry;
    }
  }

  void initializePaging(BootInformation& bootInformation)
  {
    pageDirectory = static_cast<PageDirectory*>(allocPages(bootInformation, 1, ReservedMemoryRegion::Type::PAGING));
    // Setup identity paging before enabling paging just so that we can continue
    // execution, before we transfer control to the kernel
    (*pageDirectory)[0] = PageDirectoryEntry(0, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::SUPERVISOR_ONLY, Permission::READ_WRITE, PageSize::LARGE);
    asm volatile (
        // Load page directory
        "mov cr3, %[pageDirectory];"

        // Set PSE(Page Size Extension) bit
        "mov eax, cr4;"
        "or  eax, 0x00000010;"
        "mov cr4, eax;"

        // Set PG(Paging) and PE(Protention) bit
        "mov eax, cr0;"
        "or  eax, 0x80000001;"
        "mov cr0, eax;"
        : : [pageDirectory]"r"(pageDirectory) : "eax", "memory"
        );
  }

  uintptr_t map(BootInformation& bootInformation, uintptr_t phyaddr, uintptr_t virtaddr, size_t length, common::memory::Access access, common::memory::Permission permission)
  {
    using namespace common::memory;
    rt::logf("Mapping 0x%lx to 0x%lx of length 0x%lx", phyaddr, virtaddr, length);

    if(nextVirtaddr<virtaddr+length)
      nextVirtaddr = (virtaddr+length+(PAGE_SIZE-1))&(~(PAGE_SIZE-1));

    for(size_t offset=0; offset<length; offset+=PAGE_SIZE)
    {
      const auto phyaddrLocal  = phyaddr + offset;
      const auto virtaddrLocal = virtaddr + offset;
      auto& pageTableEntry = getPageTableEntry(bootInformation, virtaddrLocal);
      if(pageTableEntry.present())
        return MAP_FAILED;

      pageTableEntry = PageTableEntry(phyaddrLocal, TLBMode::LOCAL, CacheMode::ENABLED, WriteMode::WRITE_BACK, access, permission);
    }

    return virtaddr;
  }

  uintptr_t map(BootInformation& bootInformation, uintptr_t phyaddr, size_t length, common::memory::Access access, common::memory::Permission permission)
  {
    auto virtaddr = nextVirtaddr;
    auto result = map(bootInformation, phyaddr, virtaddr, length, access, permission);
    if(result == MAP_FAILED)
      return MAP_FAILED;

    return virtaddr;
  }
}


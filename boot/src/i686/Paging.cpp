#include <boot/i686/Paging.hpp>

#include <boot/generic/Memory.hpp>
#include <boot/generic/api/Config.hpp>

#include <common/i686/memory/Paging.hpp>

#include <librt/Panic.hpp>
#include <librt/Log.hpp>
#include <librt/Iterator.hpp>
#include <librt/Assert.hpp>
#include <librt/Strings.hpp>
#include <librt/Algorithm.hpp>

namespace boot::memory
{
  extern "C" { PageDirectory* pageDirectory; }
  namespace
  {
    PageDirectory& getPageDirectory()
    {
      if(!pageDirectory)
        pageDirectory = static_cast<PageDirectory*>(allocPages(1));

      return *pageDirectory;
    }

    PageDirectoryEntry& getPageDirectoryEntry(uintptr_t virtaddr)
    {
      auto& pageDirectory = getPageDirectory();
      size_t pageDirectoryIndex = virtaddr / LARGE_PAGE_SIZE;
      auto& pageDirectoryEntry  = pageDirectory[pageDirectoryIndex];
      return pageDirectoryEntry;
    }

    PageTable& getPageTable(uintptr_t virtaddr)
    {
      auto& pageDirectoryEntry = getPageDirectoryEntry(virtaddr);
      if(!pageDirectoryEntry.present())
      {
        PageTable* pageTable = static_cast<PageTable*>(allocPages(1));
        pageDirectoryEntry = PageDirectoryEntry(reinterpret_cast<uintptr_t>(pageTable), CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::ALL, Permission::READ_WRITE);
      }
      return *reinterpret_cast<PageTable*>(pageDirectoryEntry.address());
    }

    PageTableEntry& getPageTableEntry(uintptr_t virtaddr)
    {
      auto pageTable        = getPageTable(virtaddr);
      size_t pageTableIndex = (virtaddr % LARGE_PAGE_SIZE) / PAGE_SIZE;
      auto& pageTableEntry  = pageTable[pageTableIndex];
      return pageTableEntry;
    }
  }

  uintptr_t map(uintptr_t phyaddr, uintptr_t virtaddr, size_t length, common::memory::Access access, common::memory::Permission permission)
  {
    using namespace common::memory;
    rt::logf("Mapping 0x%lx to 0x%lx of length 0x%lx\n", phyaddr, virtaddr, length);

    for(size_t offset=0; offset<length; offset+=PAGE_SIZE)
    {
      const auto phyaddrLocal  = phyaddr + offset;
      const auto virtaddrLocal = virtaddr + offset;
      auto& pageTableEntry = getPageTableEntry(virtaddrLocal);
      if(pageTableEntry.present())
        rt::panic("Attempting to modify mapped page\n");

      pageTableEntry = PageTableEntry(phyaddrLocal, TLBMode::LOCAL, CacheMode::ENABLED, WriteMode::WRITE_BACK, access, permission);
    }

    return virtaddr;
  }

  void initializePaging()
  {
    // Setup identity paging before enabling paging just so that we can continue
    // execution, before we transfer control to the kernel
    getPageDirectoryEntry(0) = PageDirectoryEntry(0, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::SUPERVISOR_ONLY, Permission::READ_WRITE, PageSize::LARGE);

    // Setup mapping for physical memory
    uintptr_t phyaddr = 0;
    for(uintptr_t virtaddr = PHYSICAL_MEMORY_MAPPING_START; virtaddr != PHYSICAL_MEMORY_MAPPING_END; virtaddr += LARGE_PAGE_SIZE)
    {
      auto& pageDirectoryEntry = getPageDirectoryEntry(virtaddr);
      pageDirectoryEntry = PageDirectoryEntry(phyaddr, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::SUPERVISOR_ONLY, Permission::READ_WRITE, PageSize::LARGE);
      phyaddr += LARGE_PAGE_SIZE;
    }
  }

  void enablePaging()
  {
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

}


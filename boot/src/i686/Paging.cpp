#include <boot/i686/Paging.hpp>

#include <boot/generic/Panic.hpp>
#include <boot/generic/Config.h>

#include <common/generic/io/Print.hpp>
#include <common/i686/memory/Paging.hpp>

#include <assert.h>
#include <string.h>

#include <algorithm>

using namespace common::memory;

namespace boot::memory
{
  // FIXME: determine this from max kernel size

  [[gnu::section(".paging")]] alignas(PAGE_SIZE) constinit PageDirectory pageDirectory;
  [[gnu::section(".paging")]] alignas(PAGE_SIZE) constinit PageTable pageTables[PAGE_TABLE_COUNT];

  PageTable* nextPageTable = pageTables;
  static PageTable* allocPageTable()
  {
    auto* result = nextPageTable++;
    if(result == std::end(pageTables))
      return nullptr;

    return result;
  }

  uintptr_t nextVirtaddr = 0;
  int map(uintptr_t phyaddr, uintptr_t virtaddr, size_t length, common::memory::Access access, common::memory::Permission permission)
  {    
    using namespace common::memory;
    common::io::printf("Mapping 0x%lx to 0x%lx of length 0x%lx", phyaddr, virtaddr, length);

    if(nextVirtaddr<virtaddr+length)
      nextVirtaddr = (virtaddr+length+(PAGE_SIZE-1))&(~(PAGE_SIZE-1));

    for(size_t offset=0; offset<length; offset+=PAGE_SIZE)
    {
      uintptr_t localPhyaddr  = phyaddr+offset;
      uintptr_t localVirtaddr = virtaddr+offset;

      size_t pageDirectoryIndex = localVirtaddr / LARGE_PAGE_SIZE;
      size_t pageTableIndex     = (localVirtaddr % LARGE_PAGE_SIZE) / PAGE_SIZE;

      auto& pageDirectoryEntry = pageDirectory[pageDirectoryIndex];
      if(!pageDirectoryEntry.present())
      {
        auto* pageTable = allocPageTable();
        if(!pageTable)
          return -1;

        pageDirectoryEntry = PageDirectoryEntry(reinterpret_cast<uintptr_t>(pageTable), CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::ALL, Permission::READ_WRITE);
      }

      auto& pageTable = *reinterpret_cast<PageTable*>(pageDirectoryEntry.address());
      auto& pageTableEntry = pageTable[pageTableIndex];
      if(pageTableEntry.present())
        return -1;

      pageTableEntry = PageTableEntry(localPhyaddr, TLBMode::LOCAL, CacheMode::ENABLED, WriteMode::WRITE_BACK, access, permission);
    }

    return 0;
  }

  uintptr_t map(uintptr_t phyaddr, size_t length, common::memory::Access access, common::memory::Permission permission)
  {
    auto virtaddr = nextVirtaddr;
    auto result = map(phyaddr, virtaddr, length, access, permission);
    if(result == -1)
      return MAP_FAILED;

    return virtaddr;
  }

  void initPaging()
  {
    // Setup identity paging before enabling paging just so that we can continue
    // execution, before we transfer control to the kernel
    pageDirectory[0] = PageDirectoryEntry(0, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::SUPERVISOR_ONLY, Permission::READ_WRITE, PageSize::LARGE);
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
        : : [pageDirectory]"r"(&pageDirectory) : "eax", "memory"
        );
  }

  int updateBootInformationPaging()
  {
    using namespace boot::memory;
    using namespace common::memory;

    addMemoryRegion(reinterpret_cast<uintptr_t>(&pageDirectory), sizeof pageDirectory, MemoryRegion::Type::PAGING);
    addMemoryRegion(reinterpret_cast<uintptr_t>(&pageTables),    reinterpret_cast<uintptr_t>(nextPageTable) - reinterpret_cast<uintptr_t>(&pageTables), MemoryRegion::Type::PAGING);

    auto result = memory::map(reinterpret_cast<uintptr_t>(pageDirectory), sizeof pageDirectory, Access::SUPERVISOR_ONLY, Permission::READ_WRITE);
    if(result == MAP_FAILED)
      return -1;

    bootInformation.pageDirectory = reinterpret_cast<PageDirectory*>(result);

    return 0;
  }
}


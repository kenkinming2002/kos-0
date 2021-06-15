#include "librt/SharedPtr.hpp"
#include "librt/UniquePtr.hpp"
#include <i686/memory/MemoryMapping.hpp>

#include <generic/BootInformation.hpp>
#include <common/i686/memory/Paging.hpp>

#include <librt/Panic.hpp>
#include <librt/Global.hpp>
#include <librt/Iterator.hpp>
#include <librt/Log.hpp>
#include <librt/Algorithm.hpp>
#include <librt/Assert.hpp>

namespace core::memory
{
  using namespace common::memory;

  constinit static rt::SharedPtr<MemoryMapping> currentMemoryMapping;

  void MemoryMapping::initialize()
  {
    physaddr_t bootPageDirectoryPhyaddr;
    asm volatile ("mov %[bootPageDirectoryPhyaddr], cr3" : [bootPageDirectoryPhyaddr]"=r"(bootPageDirectoryPhyaddr) : : "memory");
    PageDirectory* bootPageDirectory = reinterpret_cast<PageDirectory*>(physToVirt(bootPageDirectoryPhyaddr));

    currentMemoryMapping = rt::makeShared<MemoryMapping>(bootPageDirectory);
  }

  rt::SharedPtr<MemoryMapping> MemoryMapping::current()
  {
    return currentMemoryMapping;
  }

  void MemoryMapping::makeCurrent(rt::SharedPtr<MemoryMapping> memoryMapping)
  {
    asm volatile ("mov cr3, %[pageDirectoryPhyaddr]" : : [pageDirectoryPhyaddr]"r"(virtToPhys(reinterpret_cast<uintptr_t>(memoryMapping->m_pageDirectory))) : "memory");
    currentMemoryMapping = rt::move(memoryMapping);
  }

  rt::SharedPtr<MemoryMapping> MemoryMapping::allocate()
  {
    auto page = allocPages(1);
    if(!page)
      return nullptr;

    const auto& currentPageDirectory = *current()->m_pageDirectory;
    auto& pageDirectory = *reinterpret_cast<common::memory::PageDirectory*>(page->address());

    rt::fill(rt::begin(pageDirectory), rt::end(pageDirectory), common::memory::PageDirectoryEntry());
    rt::copy(&currentPageDirectory[768], &currentPageDirectory[1024], &pageDirectory[768]);

    return rt::makeShared<MemoryMapping>(&pageDirectory);
  }

  MemoryMapping::MemoryMapping(common::memory::PageDirectory* pageDirectory) : m_pageDirectory(pageDirectory) {}
  MemoryMapping::~MemoryMapping()
  {
    auto page = Pages::from(reinterpret_cast<uintptr_t>(m_pageDirectory), PAGE_SIZE);
    freePages(page);
  }

  void MemoryMapping::map(Pages virtualPages, common::memory::Access access, common::memory::Permission permission, rt::Optional<Pages> physicalPages)
  {
    using namespace common::memory;

    ASSERT(!physicalPages || physicalPages->count == virtualPages.count);
    for(size_t i=0; i<virtualPages.count; ++i)
    {
      size_t virtualIndex = virtualPages.index+i;
      auto& pageTableEntry = this->pageTableEntry(virtualIndex, true/*allocate*/);
      if(pageTableEntry.present())
        rt::panic("Attempting to map pages that have already been mapped\n");

      if(physicalPages)
      {
        pageTableEntry = PageTableEntry(physicalPages->address() + i * PAGE_SIZE, TLBMode::LOCAL, CacheMode::ENABLED, WriteMode::WRITE_BACK, access, permission);
      }
      else
      {
        auto pages = allocPages(1);
        if(!pages)
          rt::panic("Out of physical pages\n");

        pageTableEntry = PageTableEntry(virtToPhys(pages->address()), TLBMode::LOCAL, CacheMode::ENABLED, WriteMode::WRITE_BACK, access, permission);
      }

      asm volatile ( "invlpg [%[virtaddr]]" : : [virtaddr]"r"(virtualIndex * PAGE_SIZE) : "memory");
    }
  }

  void MemoryMapping::unmap(Pages virtualPages)
  {
    using namespace common::memory;

    for(size_t i=0; i<virtualPages.count; ++i)
    {
      size_t virtualIndex = virtualPages.index+i;
      auto& pageTableEntry = this->pageTableEntry(virtualIndex, false/*allocate*/);
      if(!pageTableEntry.present())
        rt::panic("Attempting to unmap pages that are not mapped");

      auto pages = Pages::from(physToVirt(pageTableEntry.address()), PAGE_SIZE);
      pageTableEntry = PageTableEntry();

      freePages(pages);
    }
  }

  common::memory::PageDirectoryEntry& MemoryMapping::pageDirectoryEntry(size_t virtualIndex, bool allocate)
  {
    using namespace common::memory;

    size_t pageDirectoryIndex = virtualIndex / 1024;
    auto& pageDirectory       = *m_pageDirectory;
    auto& pageDirectoryEntry  = pageDirectory[pageDirectoryIndex];
    if(!pageDirectoryEntry.present())
    {
      if(!allocate)
        rt::panic("Page Directory Entry not present\n");

      auto pages = allocPages(1);
      if(!pages)
        rt::panic("Out of physical pages");

      auto& pageTable = *reinterpret_cast<common::memory::PageTable*>(pages->address());
      rt::fill(rt::begin(pageTable), rt::end(pageTable), PageTableEntry());

      pageDirectoryEntry = PageDirectoryEntry(virtToPhys(pages->address()), CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::ALL, Permission::READ_WRITE);
    }

    return pageDirectoryEntry;
  }

  common::memory::PageTableEntry& MemoryMapping::pageTableEntry(size_t virtualIndex, bool allocate)
  {
    using namespace common::memory;

    auto& pageDirectoryEntry = this->pageDirectoryEntry(virtualIndex, allocate);

    size_t pageTableIndex = virtualIndex % 1024;
    auto& pageTable       = *reinterpret_cast<common::memory::PageTable*>(physToVirt(pageDirectoryEntry.address()));
    auto& pageTableEntry  = pageTable[pageTableIndex];

    return pageTableEntry;
  }
}


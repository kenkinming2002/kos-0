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

  constinit static PageDirectoryEntry kernelPageDirectoryEntries[256];

  constinit static rt::Global<MemoryMapping> initialMemoryMapping;
  constinit static MemoryMapping* currentMemoryMapping;

  void MemoryMapping::initialize()
  {
    const auto& bootPageDirectory = *bootInformation->pageDirectory;
    rt::copy(&bootPageDirectory[768], &bootPageDirectory[1024], rt::begin(kernelPageDirectoryEntries));

    initialMemoryMapping.construct(bootInformation->pageDirectory);
    currentMemoryMapping = &initialMemoryMapping();
  }

  MemoryMapping& MemoryMapping::current()
  {
    return *currentMemoryMapping;
  }

  rt::UniquePtr<MemoryMapping> MemoryMapping::allocate()
  {
    auto page = allocMappedPages(1);
    if(!page)
      return nullptr;

    auto& pageDirectory = *reinterpret_cast<common::memory::PageDirectory*>(page->address());
    rt::fill(rt::begin(pageDirectory), rt::end(pageDirectory), common::memory::PageDirectoryEntry());

    auto memoryMapping = rt::makeUnique<MemoryMapping>(&pageDirectory);
    memoryMapping->synchronize();
    return memoryMapping;
  }

  MemoryMapping::~MemoryMapping()
  {
    auto page = Pages::from(reinterpret_cast<uintptr_t>(m_pageDirectory), PAGE_SIZE);
    freeMappedPages(page);
  }

  uintptr_t MemoryMapping::doFractalMapping(uintptr_t phyaddr, size_t length)
  {
    using namespace common::memory;

    // All the magic happen here
    const size_t    pageDirectoryIndex = 1023;
    const uintptr_t phyaddrBase  = phyaddr & ~(LARGE_PAGE_SIZE-1);
    const uintptr_t virtaddrBase = LARGE_PAGE_SIZE * pageDirectoryIndex;

    auto& pageDirectory = *current().m_pageDirectory;
    auto& pageDirectoryEntry = pageDirectory[pageDirectoryIndex];
    if(!pageDirectoryEntry.present() || pageDirectoryEntry.address() != phyaddrBase)
    {
      pageDirectoryEntry = PageDirectoryEntry(phyaddrBase, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::SUPERVISOR_ONLY, Permission::READ_WRITE, PageSize::LARGE);
      asm volatile ( "invlpg [0xFFC00000]" : : : "memory");
    }

    // There is some clever bit-masking you can do but who cares?
    return virtaddrBase+(phyaddr-phyaddrBase);
  }

  void MemoryMapping::synchronize()
  {
    auto& pageDirectory = *m_pageDirectory;
    rt::copy(rt::begin(kernelPageDirectoryEntries), rt::end(kernelPageDirectoryEntries), &pageDirectory[768]);
  }

  void MemoryMapping::makeCurrent()
  {
    // Synchronize. We may be able to implement a page fault handler for this,
    // but that is too much trouble, and it is uncertain whether that will
    // actually bring any benefit considering that the cost of page fault can be
    // pretty high. Premature optimization is the root of all evil they say.
    synchronize();
    asm volatile ("mov cr3, %[pageDirectoryPhysicalAddress]" : : [pageDirectoryPhysicalAddress]"r"(virtualToPhysical(reinterpret_cast<uintptr_t>(m_pageDirectory))) : "memory");
    currentMemoryMapping = this;
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
        auto physicalPage = allocPhysicalPages(1);
        if(!physicalPage)
          rt::panic("Out of physical pages\n");

        pageTableEntry = PageTableEntry(physicalPage->address(), TLBMode::LOCAL, CacheMode::ENABLED, WriteMode::WRITE_BACK, access, permission);
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

      auto physicalPages = Pages::from(pageTableEntry.address(), PAGE_SIZE);
      pageTableEntry = PageTableEntry();

      freePhysicalPages(physicalPages);
    }
  }

  uintptr_t MemoryMapping::virtualToPhysical(uintptr_t virtaddr)
  {
    auto& pageTableEntry = this->pageTableEntry(virtaddr / PAGE_SIZE, false/*allocate*/);
    if(!pageTableEntry.present())
      rt::panic("Page Table Entry not present\n");

    return pageTableEntry.address();
  }

  common::memory::PageDirectoryEntry& MemoryMapping::pageDirectoryEntry(size_t virtualIndex, bool allocate)
  {
    using namespace common::memory;

    size_t pageDirectoryIndex = virtualIndex / 1024;
    auto& pageDirectory       = *m_pageDirectory;
    auto& pageDirectoryEntry  = pageDirectory[pageDirectoryIndex];
    if(pageDirectoryIndex>=768)
      pageDirectoryEntry = kernelPageDirectoryEntries[pageDirectoryIndex-768];

    if(!pageDirectoryEntry.present())
    {
      if(!allocate)
        rt::panic("Page Directory Entry not present\n");

      auto physicalPage = allocPhysicalPages(1);
      if(!physicalPage)
        rt::panic("Out of physical pages");

      pageDirectoryEntry = PageDirectoryEntry(physicalPage->address(), CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::ALL, Permission::READ_WRITE);
      if(pageDirectoryIndex>=768)
      {
        auto& kernelPageDirectoryEntry = kernelPageDirectoryEntries[pageDirectoryIndex-768];
        kernelPageDirectoryEntry = PageDirectoryEntry(physicalPage->address(), CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::ALL, Permission::READ_WRITE);
      }

      auto& pageTable = *reinterpret_cast<common::memory::PageTable*>(doFractalMapping(pageDirectoryEntry.address(), PAGE_SIZE));
      rt::fill(rt::begin(pageTable), rt::end(pageTable), PageTableEntry());
    }

    return pageDirectoryEntry;
  }

  common::memory::PageTableEntry& MemoryMapping::pageTableEntry(size_t virtualIndex, bool allocate)
  {
    using namespace common::memory;

    auto& pageDirectoryEntry = this->pageDirectoryEntry(virtualIndex, allocate);

    size_t pageTableIndex = virtualIndex % 1024;
    auto& pageTable       = *reinterpret_cast<common::memory::PageTable*>(doFractalMapping(pageDirectoryEntry.address(), PAGE_SIZE));
    auto& pageTableEntry  = pageTable[pageTableIndex];

    return pageTableEntry;
  }
}


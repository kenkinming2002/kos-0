#include <core/i686/memory/MemoryMapping.hpp>

#include <boot/i686/Boot.hpp>

#include <core/generic/Init.hpp>
#include <core/generic/Panic.hpp>
#include <core/generic/io/Print.hpp>

#include <assert.h>
#include <algorithm>

namespace core::memory
{
  INIT_NOLOG MemoryMapping initialMemoryMapping = MemoryMapping(reinterpret_cast<uintptr_t>(initialPageDirectory)-0xC0000000, reinterpret_cast<uintptr_t>(initialPageDirectory)); // Ideally, this should be constinit, but reinterpret_cast is not allowed here
  constinit MemoryMapping* MemoryMapping::current = &initialMemoryMapping;

  constinit common::memory::PageDirectoryEntry MemoryMapping::kernelPageDirectoryEntries[256];

  INIT_FUNCTION_NOLOG void initMemoryMapping()
  {
    std::copy(&initialMemoryMapping.pageDirectory()[768], &initialMemoryMapping.pageDirectory()[1024], std::begin(MemoryMapping::kernelPageDirectoryEntries));
  }


  std::optional<MemoryMapping> MemoryMapping::allocate()
  {
    auto page = allocMappedPages(1);
    if(!page)
      return std::nullopt;

    auto virtualAddress  = page->address();
    auto physicalAddress = current->virtualToPhysical(virtualAddress); // The page table should have just be mapped in so this should be fast

    auto memoryMapping = MemoryMapping(physicalAddress, virtualAddress);
    std::fill(&memoryMapping.pageDirectory()[0], &memoryMapping.pageDirectory()[768], common::memory::PageDirectoryEntry());
    std::copy(std::begin(kernelPageDirectoryEntries), std::end(kernelPageDirectoryEntries), &memoryMapping.pageDirectory()[768]);

    return memoryMapping;
  }

  MemoryMapping::MemoryMapping(uintptr_t pageDirectoryPhysicalAddress, uintptr_t pageDirectoryVirtualAddress)
    : m_pageDirectoryPhysicalAddress(pageDirectoryPhysicalAddress), m_pageDirectoryVirtualAddress(pageDirectoryVirtualAddress) {}

  MemoryMapping::~MemoryMapping()
  {
    if(m_pageDirectoryVirtualAddress)
    {
      auto page = Pages::from(m_pageDirectoryVirtualAddress, PAGE_SIZE);
      freeMappedPages(page);
    }
  }

  MemoryMapping::MemoryMapping(MemoryMapping&& other) 
    : m_pageDirectoryPhysicalAddress(std::exchange(other.m_pageDirectoryPhysicalAddress, 0)), 
      m_pageDirectoryVirtualAddress(std::exchange(other.m_pageDirectoryVirtualAddress, 0)) {}

  MemoryMapping& MemoryMapping::operator=(MemoryMapping&& other) 
  {
    std::swap(m_pageDirectoryPhysicalAddress, other.m_pageDirectoryPhysicalAddress);
    std::swap(m_pageDirectoryVirtualAddress, other.m_pageDirectoryVirtualAddress);
    return *this;
  }

  uintptr_t MemoryMapping::doFractalMapping(uintptr_t phyaddr, size_t length)
  {
    using namespace common::memory;

    // All the magic happen here
    const size_t    pageDirectoryIndex = 1023;
    const uintptr_t phyaddrBase  = phyaddr & ~(LARGE_PAGE_SIZE-1);
    const uintptr_t virtaddrBase = LARGE_PAGE_SIZE * pageDirectoryIndex;

    auto& pageDirectoryEntry = current->pageDirectory()[pageDirectoryIndex];
    if(!pageDirectoryEntry.present() || pageDirectoryEntry.address() != phyaddrBase)
    {
      pageDirectoryEntry = PageDirectoryEntry(phyaddrBase, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::SUPERVISOR_ONLY, Permission::READ_WRITE, PageSize::LARGE);
      asm volatile ( "invlpg [0xFFC00000]" : : : "memory");
    }

    // There is some clever bit-masking you can do but who cares?
    return virtaddrBase+(phyaddr-phyaddrBase);
  }

  void MemoryMapping::makeCurrent()
  {
    current = this;

    // Synchronize. We may be able to implement a page fault handler for this,
    // but that is too much trouble, and it is uncertain whether that will
    // actually bring any benefit considering that the cost of page fault can be
    // pretty high. Premature optimization is the root of all evil they say.
    std::copy(std::begin(kernelPageDirectoryEntries), std::end(kernelPageDirectoryEntries), &pageDirectory()[768]); 
    asm volatile ("mov cr3, %[pageDirectoryPhysicalAddress]" : : [pageDirectoryPhysicalAddress]"r"(m_pageDirectoryPhysicalAddress) : "memory");
  }

  void MemoryMapping::map(Pages physicalPages, Pages virtualPages, common::memory::Access access, common::memory::Permission permission)
  {
    assert(physicalPages.count == virtualPages.count);

    using namespace common::memory;

    for(size_t i=0; i<physicalPages.count; ++i)
    {
      size_t virtualIndex = virtualPages.index+i;
      size_t pageDirectoryIndex = virtualIndex / 1024;
      size_t pageTableIndex     = virtualIndex % 1024;

      auto& pageDirectoryEntry = pageDirectory()[pageDirectoryIndex];
      if(!pageDirectoryEntry.present())
      {
        auto physicalPage = allocPhysicalPages(1);
        if(!physicalPage)
          panic("Out of physical pages");

        pageDirectoryEntry = PageDirectoryEntry(physicalPage->address(), CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::ALL, Permission::READ_WRITE);
        if(pageDirectoryIndex>=768)
        {
          auto& kernelPageDirectoryEntry = kernelPageDirectoryEntries[pageDirectoryIndex-768];
          kernelPageDirectoryEntry = PageDirectoryEntry(physicalPage->address(), CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::ALL, Permission::READ_WRITE);
        }
      }

      auto& pageTable      = *reinterpret_cast<common::memory::PageTable*>(doFractalMapping(pageDirectoryEntry.address(), PAGE_SIZE));
      auto& pageTableEntry = pageTable[pageTableIndex];

      pageTableEntry       = PageTableEntry(physicalPages.address() + i * PAGE_SIZE , TLBMode::LOCAL, CacheMode::ENABLED, WriteMode::WRITE_BACK, access, permission);
      asm volatile ( "invlpg [%[virtaddr]]" : : [virtaddr]"r"(virtualIndex * PAGE_SIZE) : "memory");
    }
  }

  Pages MemoryMapping::unmap(Pages virtualPages)
  {
    using namespace common::memory;

    uintptr_t phyaddr = 0;
    for(size_t i=0; i<virtualPages.count; ++i)
    {
      size_t virtualIndex = virtualPages.index+i;
      size_t pageDirectoryIndex = virtualIndex / 1024;
      size_t pageTableIndex     = virtualIndex % 1024;

      auto& pageDirectoryEntry = pageDirectory()[pageDirectoryIndex];
      if(!pageDirectoryEntry.present())
        panic("Attempting to unmap pages that are not mapped");

      auto& pageTable      = *reinterpret_cast<common::memory::PageTable*>(doFractalMapping(pageDirectoryEntry.address(), PAGE_SIZE));
      auto& pageTableEntry = pageTable[pageTableIndex];

      if(!pageTableEntry.present())
        panic("Attempting to unmap pages that are not mapped");

      if(i==0)
        phyaddr = pageTableEntry.address();

      pageTableEntry = PageTableEntry();
      if(pageDirectoryIndex>=768)
      {
        auto& kernelPageDirectoryEntry = kernelPageDirectoryEntries[pageDirectoryIndex-768];
        kernelPageDirectoryEntry = PageDirectoryEntry();
      }
      asm volatile ( "invlpg [%[virtaddr]]" : : [virtaddr]"r"(virtualIndex * PAGE_SIZE) : "memory");
    }

    return Pages::from(phyaddr, virtualPages.count * PAGE_SIZE);
  }

  uintptr_t MemoryMapping::virtualToPhysical(uintptr_t virtaddr)
  {
    if(this != current)
      panic("Warning");

    size_t pageDirectoryIndex = (virtaddr / PAGE_SIZE) / 1024;
    size_t pageTableIndex     = (virtaddr / PAGE_SIZE) % 1024;

    const auto& pageDirectoryEntry = pageDirectory()[pageDirectoryIndex];
    if(!pageDirectoryEntry.present())
      panic("Page Directory not present"); // TODO: We should also look in kernelPageDirectory

    const auto& pageTable          = *reinterpret_cast<common::memory::PageTable*>(doFractalMapping(pageDirectoryEntry.address(), PAGE_SIZE)); // TODO: Error checking
    const auto& pageTableEntry     = pageTable[pageTableIndex];

    return pageTableEntry.address();
  }
}


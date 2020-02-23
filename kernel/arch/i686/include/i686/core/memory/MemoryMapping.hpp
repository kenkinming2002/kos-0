#pragma once

#include <i686/core/memory/Paging.hpp>
  
namespace core::memory
{
  class MemoryMapping;
  class HigherHalfMemoryMapping;

  class MemoryMapping
  {
  public:
    MemoryMapping();
    MemoryMapping(PageDirectory& pageDirectory, phyaddr_t pageDirectoryPhysicalAddress);
    MemoryMapping(const HigherHalfMemoryMapping& higherHalfMemoryMapping);

  public:
    void map(MemoryRegion physicalMemoryRegion, MemoryRegion virtualMemoryRegion, Access access, Permission permission);
    /*
     * Unmap a virtual memory region
     *
     * @return physical memory region previously mapped to virtualMemoryRegion
     */
    MemoryRegion unmap(MemoryRegion virtualMemoryRegion);

  public:
    /*
     * Set this memory mapping as the active memory mapping
     *
     * @param self physical address of this
     */
    void setAsActive() const;

  public:
    const PageDirectory& pageDirectory() const { return *m_pageDirectory; }

  private:
    virtaddr_t doFractalMapping(phyaddr_t physicalAddress);

  private:
    PageDirectory* m_pageDirectory;
    phyaddr_t m_pageDirectoryPhysicalAddress;
  };

  class HigherHalfMemoryMapping
  {
  public:
    HigherHalfMemoryMapping(const MemoryMapping& memoryMapping);

  public:
    PageDirectoryEntry pageDirectoryEntires[256];
  };

  extern MemoryMapping currentMemoryMapping;
  extern HigherHalfMemoryMapping higherHalfMemoryMapping;
}

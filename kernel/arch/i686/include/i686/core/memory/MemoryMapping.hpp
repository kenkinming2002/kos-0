#pragma once

#include <i686/core/memory/Paging.hpp>
  
namespace core::memory
{
  void initMemoryMapping();

  class MemoryMapping
  {
  public:
    MemoryMapping();
    MemoryMapping(PageDirectory& pageDirectory, phyaddr_t pageDirectoryPhysicalAddress);

  public:
    static void init();

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

  private:
    virtaddr_t doFractalMapping(phyaddr_t physicalAddress);

  private:
    PageDirectory* m_pageDirectory;
    phyaddr_t m_pageDirectoryPhysicalAddress;

  private:
    // Page Directory Entries for higher half, shared among all MemoryMapping
    static PageDirectoryEntry kernelPageDirectoryEntires[256];
  };

  extern MemoryMapping currentMemoryMapping;
}

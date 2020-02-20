#include <i686/core/memory/Paging.hpp>

namespace core::memory
{
  PageDirectoryEntry::PageDirectoryEntry() : m_data(0) {}
  PageDirectoryEntry::PageDirectoryEntry(uint32_t address, CacheMode cacheMode,
      WriteMode writeMode, Access access, Permission permission, PageSize pageSize)
    : m_data(0)
  {
    m_data |= address & 0xFFFFF000;
    if(pageSize   == PageSize::LARGE)          m_data |= 1u<<7;
    if(cacheMode  == CacheMode::DISABLED)      m_data |= 1u<<4;
    if(writeMode  == WriteMode::WRITE_THROUGH) m_data |= 1u<<3;
    if(access     == Access::ALL)              m_data |= 1u<<2;
    if(permission == Permission::READ_WRITE)   m_data |= 1u<<1;
    m_data |= 1u; // Present
  }
  
  bool PageDirectoryEntry::present() const
  {
    return m_data & 1u;
  }

  uint32_t PageDirectoryEntry::address() const
  {
    return m_data & 0xFFFFF000;
  }

  void PageDirectoryEntry::present(bool present)
  {
    m_data = (m_data & ~1u) | static_cast<uint32_t>(present);
  }

  void PageDirectoryEntry::address(uint32_t address)
  {
    m_data = (m_data & ~0xFFFFF000) | static_cast<uint32_t>(address);
  }

  PageTableEntry::PageTableEntry() : m_data(0) {}
  PageTableEntry::PageTableEntry(uint32_t address, TLBMode tlbMode, CacheMode cacheMode, WriteMode writeMode, Access access, 
      Permission permission)
  {
    m_data = address & 0xFFFFF000;
    if(tlbMode    == TLBMode::GLOBAL)          m_data |= 1u<<8;
    if(cacheMode  == CacheMode::DISABLED)      m_data |= 1u<<4;
    if(writeMode  == WriteMode::WRITE_THROUGH) m_data |= 1u<<3;
    if(access     == Access::ALL)              m_data |= 1u<<2;
    if(permission == Permission::READ_WRITE)   m_data |= 1u<<1;
    m_data |= 1u; // Present
  }

  bool PageTableEntry::present() const
  {
    return m_data & 1u;
  }

  uint32_t PageTableEntry::address() const
  {
    return m_data & 0xFFFFF000;
  }

  void PageTableEntry::present(bool present)
  {
    m_data = (m_data & ~1u) | static_cast<uint32_t>(present);
  }

  void PageTableEntry::address(uint32_t address)
  {
    m_data = (m_data & ~0xFFFFF000) | static_cast<uint32_t>(address);
  }


  MemoryMapping::MapResult MemoryMapping::map(MemoryRegion physicalMemoryRegion, MemoryRegion virtualMemoryRegion, 
      void* pageTablePhysicalMemory)
  {
    // NOTE : index = pageDirectoryIndex * PAGE_TABLE_ENTRY_COUNT +
    //                pageTableIndex
    auto pageDirectoryIndex = virtualMemoryRegion.beginIndex() / PAGE_TABLE_ENTRY_COUNT;
    auto pageTableIndex     = virtualMemoryRegion.beginIndex() % PAGE_TABLE_ENTRY_COUNT;

    auto& pageDirectoryEntry = pageDirectory[pageDirectoryIndex];

    // 2: Obtain physical Address to target page table
    phyaddr_t pageTablePhysicalAddress;
    if(!pageDirectoryEntry.present())
    {
      if(pageTablePhysicalMemory)
        pageTablePhysicalAddress = reinterpret_cast<phyaddr_t>(pageTablePhysicalMemory);
      else
        return MapResult::ERR_NO_PAGE_TABLE;
    }
    else
      pageTablePhysicalAddress = pageDirectoryEntry.address();

    auto& pageTable = utils::deref_cast<PageTable>(doFractalMapping(pageTablePhysicalAddress));

    // 3: Clear page table to zero if it is new
    if(!pageDirectoryEntry.present())
    {
      for(PageTableEntry& pageTableEntry: pageTable)
        pageTableEntry = PageTableEntry();
    }

    // 3: Write to Page Table through Fractal Mapping
    for(size_t i = 0; i<virtualMemoryRegion.count(); ++i)
    {
      //auto physicalPageFrame = &physicalPageFrames[i];
      auto physicalAddress = physicalMemoryRegion.begin() + i * PAGE_SIZE;

      auto pageTableEntry = PageTableEntry(physicalAddress, TLBMode::LOCAL, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::SUPERVISOR_ONLY, Permission::READ_WRITE);
      if(pageTableIndex+i<1024)
        pageTable[pageTableIndex+i] = pageTableEntry;
      else
        for(;;) asm("hlt"); // Page Table Overrun, Unimplemented
    }

    // 4: Update Page Directory Entry to point to the possibly new Page Table
    if(!pageDirectoryEntry.present())
      pageDirectoryEntry = PageDirectoryEntry(pageTablePhysicalAddress, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::SUPERVISOR_ONLY, Permission::READ_WRITE);


    asm volatile ( R"(
      .intel_syntax noprefix
        mov eax, cr3
        mov cr3, eax
      .att_syntax prefix
      )"
      :
      : 
      : "eax"
    );

    return MapResult::SUCCESS;
  }
  /*
   * Unmap a virtual memory region
   *
   * @return physical memory region previously mapped to virtualMemoryRegion
   */
  MemoryRegion MemoryMapping::unmap(MemoryRegion virtualMemoryRegion)
  {
    // NOTE : index = pageDirectoryIndex * PAGE_TABLE_ENTRY_COUNT +
    //                pageTableIndex
    auto pageDirectoryIndex = virtualMemoryRegion.beginIndex() / PAGE_TABLE_ENTRY_COUNT;
    auto pageTableIndex     = virtualMemoryRegion.beginIndex() % PAGE_TABLE_ENTRY_COUNT;

    auto& pageDirectoryEntry = pageDirectory[pageDirectoryIndex];

    if(!pageDirectoryEntry.present())
        for(;;) asm("hlt"); // WTH, unmapping a region that is never mapped!?

    phyaddr_t pageTablePhysicalAddress = pageDirectoryEntry.address();
    auto& pageTable = utils::deref_cast<PageTable>(doFractalMapping(pageTablePhysicalAddress));

    // Take the address mapped by first page table as the starting address for mapped PhysicalPageFrameRange
    auto physicalAddressMapped = pageTable[pageTableIndex].address(); 

    // Unset all Page Table Entry through Fractal Mapping
    for(size_t i = 0; i<virtualMemoryRegion.count(); ++i)
    {
      if(pageTableIndex+i<1024)
      {
        auto& pageTableEntry = pageTable[pageTableIndex+i];
        pageTableEntry.present(false);
        pageTableEntry.address(0u); // For consistency only
      }
      else
        for(;;) asm("hlt"); // Page Table Overrun, Unimplemented
    }

    asm volatile ( R"(
      .intel_syntax noprefix
        mov eax, cr3
        mov cr3, eax
      .att_syntax prefix
      )"
      :
      : 
      : "eax"
    );

    // NOTE: There is no need to free the page table, we can reuse it next time

    return MemoryRegion(physicalAddressMapped / PAGE_SIZE, virtualMemoryRegion.count(), MemoryRegion::index_length_tag);
  }

  virtaddr_t MemoryMapping::doFractalMapping(phyaddr_t physicalAddress)
  {
    // Write to last Page Directory
    uintptr_t alignedPhysicalAddress = physicalAddress & 0xFFC00000; // Aligned to 4MiB mark
    pageDirectory[PAGE_DIRECTORY_ENTRY_COUNT-1] = PageDirectoryEntry(alignedPhysicalAddress, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::SUPERVISOR_ONLY, Permission::READ_WRITE, PageSize::LARGE);

    virtaddr_t fractalMappingAddress = (PAGE_DIRECTORY_ENTRY_COUNT-1) * PAGE_TABLE_ENTRY_COUNT * PAGE_SIZE;
    
    asm volatile ( R"(
      .intel_syntax noprefix
        mov eax, cr3
        mov cr3, eax
      .att_syntax prefix
      )"
      :
      : 
      : "eax"
    );

    // Add back the discarded bit due to 4Mib alignment
    return fractalMappingAddress + (physicalAddress & 0x003FFFFF);
  }
}

#include <i686/core/memory/MemoryMapping.hpp>

#include <algorithm>
#include <generic/core/Memory.hpp>

#include <i686/core/Interrupt.hpp>
#include <generic/io/Print.hpp>

namespace core::memory
{
  /******************************************
   * Initialization and Page Fault Handling *
   ******************************************/
  namespace
  {
    [[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void page_fault_handler(core::interrupt::frame*, 
        core::interrupt::uword_t error)
    {
      CORE_INTERRUPT_ENTRY;

      uintptr_t faultingAddress;
      asm volatile ( R"(
        .intel_syntax noprefix
        mov %[faultingAddress], cr2
        .att_syntax prefix
        )"
        : [faultingAddress]"=r"(faultingAddress)
        : 
        :
      );

      io::print("Page Fault:\n");
      io::print("  - error code: ", (uint32_t)error, "\n");
      io::print("  - at: ", faultingAddress, "\n");

      if(faultingAddress>=0xC0000000) // Higher Half Page Fault
      {
        for(;;) asm("hlt");
      }

      for(;;) asm("hlt");

      CORE_INTERRUPT_EXIT;
    } 
  }


  void initMemoryMapping()
  {
    core::interrupt::install_handler(0xE, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&page_fault_handler));
    MemoryMapping::init();
  }

  PageDirectoryEntry MemoryMapping::kernelPageDirectoryEntires[256];

  MemoryMapping::MemoryMapping()
  {
    {
      static_assert(sizeof(PageDirectory) == core::memory::PAGE_SIZE);

      // TODO: allocate a zeroed page directly
      auto [pages, physicalAddress] = core::memory::allocateHeapPages(1);
      if(!pages)
        for(;;) asm("hlt"); // OOM

      m_pageDirectory = static_cast<PageDirectory*>(pages);
      m_pageDirectoryPhysicalAddress = physicalAddress;
    }

    auto& pageDirectory = *m_pageDirectory;

    // Zero the lower half
    for(size_t i=0; i<768; ++i)
      pageDirectory[i].clear();

    // Copy the higher half from kernel page directory
    for(size_t i=0; i<256; ++i)
      pageDirectory[i+768] = kernelPageDirectoryEntires[i];
  }

  MemoryMapping::MemoryMapping(PageDirectory& pageDirectory, phyaddr_t pageDirectoryPhysicalAddress)
    : m_pageDirectory(&pageDirectory), m_pageDirectoryPhysicalAddress(pageDirectoryPhysicalAddress) {}

  void MemoryMapping::init()
  {
    auto& kernelPageDirectory = utils::deref_cast<PageDirectory>(::kernelPageDirectory);
    std::copy(&kernelPageDirectory[768], &kernelPageDirectory[1024], kernelPageDirectoryEntires);
  }

  void MemoryMapping::map(MemoryRegion physicalMemoryRegion, MemoryRegion virtualMemoryRegion, Access access, Permission permission)
  {
    // NOTE : index = pageDirectoryIndex * PAGE_TABLE_ENTRY_COUNT +
    //                pageTableIndex
    auto pageDirectoryIndex = virtualMemoryRegion.beginIndex() / PAGE_TABLE_ENTRY_COUNT;
    auto pageTableIndex     = virtualMemoryRegion.beginIndex() % PAGE_TABLE_ENTRY_COUNT;

    auto& pageDirectoryEntry = (*m_pageDirectory)[pageDirectoryIndex];

    // 2: Obtain physical Address to target page table
    phyaddr_t pageTablePhysicalAddress;
    if(!pageDirectoryEntry.present())
    {
      if(auto physicalMemoryRegion = core::memory::allocatePhysicalMemoryRegion(1))
        pageTablePhysicalAddress = reinterpret_cast<uintptr_t>(physicalMemoryRegion->begin());
      else
        for(;;) asm("hlt"); // OOM
    }
    else
      pageTablePhysicalAddress = pageDirectoryEntry.address();

    auto& pageTable = utils::deref_cast<PageTable>(doFractalMapping(pageTablePhysicalAddress));

    // 3: Clear page table to zero if it is new
    if(!pageDirectoryEntry.present())
    {
      for(PageTableEntry& pageTableEntry: pageTable)
        pageTableEntry.clear();
    }

    // 3: Write to Page Table through Fractal Mapping
    for(size_t i = 0; i<virtualMemoryRegion.count(); ++i)
    {
      //auto physicalPageFrame = &physicalPageFrames[i];
      auto physicalAddress = physicalMemoryRegion.begin() + i * PAGE_SIZE;

      auto pageTableEntry = 
        PageTableEntry(physicalAddress, TLBMode::LOCAL, CacheMode::ENABLED, WriteMode::WRITE_BACK, access, permission);

      if(pageTableIndex+i<1024)
        pageTable[pageTableIndex+i] = pageTableEntry;
      else
        for(;;) asm("hlt"); // Page Table Overrun, Unimplemented
    }

    // 4: Update Page Directory Entry to point to the possibly new Page Table if(!pageDirectoryEntry.present())
    {
      pageDirectoryEntry = 
        PageDirectoryEntry(pageTablePhysicalAddress, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::ALL, Permission::READ_WRITE);
      // Update higherHalfMemoryMapping if we are writing to higher half
      if(pageDirectoryIndex>=768)
      {
        kernelPageDirectoryEntires[pageDirectoryIndex-768] = 
          PageDirectoryEntry(pageTablePhysicalAddress, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::ALL, Permission::READ_WRITE);
      }
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

    auto& pageDirectoryEntry = (*m_pageDirectory)[pageDirectoryIndex];

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
        pageTableEntry.clear();
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
    (*m_pageDirectory)[PAGE_DIRECTORY_ENTRY_COUNT-1] = PageDirectoryEntry(alignedPhysicalAddress, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::SUPERVISOR_ONLY, Permission::READ_WRITE, PageSize::LARGE);

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

  void MemoryMapping::setAsActive() const
  {
    asm volatile ( R"(
      .intel_syntax noprefix
        mov cr3, %[pageDirectory]
      .att_syntax prefix
      )"
      :
      : [pageDirectory]"r"(m_pageDirectoryPhysicalAddress)
      :
    );
    currentMemoryMapping = *this;
  }

  MemoryMapping currentMemoryMapping = MemoryMapping(utils::deref_cast<PageDirectory>(kernelPageDirectory), reinterpret_cast<uintptr_t>(kernelPageDirectory) - 0xC0000000);
}

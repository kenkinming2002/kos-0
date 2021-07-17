#include <i686/memory/MemoryMapping.hpp>

#include <generic/BootInformation.hpp>
#include <generic/tasks/Scheduler.hpp>
#include <i686/interrupts/Interrupts.hpp>

#include <librt/Panic.hpp>
#include <librt/Global.hpp>
#include <librt/Iterator.hpp>
#include <librt/Log.hpp>
#include <librt/Algorithm.hpp>
#include <librt/Assert.hpp>

namespace core::memory
{
  /* A Big question, how should we do memory reclaim, and how should we respond
   * to out of memory situtation */
  namespace
  {
    void pageFaultHandler(irq_t irqNumber, uword_t errorCode, uintptr_t oldEip)
    {
      uintptr_t addr;
      asm volatile ("mov %[address], cr2" : [address]"=rm"(addr) : :);
      auto result = MemoryMapping::current->handlePageFault(addr, errorCode);
      if(!result)
      {
        rt::logf("process killed by page fault at 0x%lx with error code:0x%lx and old eip:0x%lx\n", addr, errorCode, oldEip);
        tasks::killCurrent(-1);
      }
    }
  }

  constinit rt::SharedPtr<MemoryMapping> MemoryMapping::current;

  void MemoryMapping::initialize()
  {
    interrupts::uninstallHandler(14);
    interrupts::installHandler(14, &pageFaultHandler, PrivilegeLevel::RING0, true);

    physaddr_t bootPageDirectoryPhyaddr;
    asm volatile ("mov %[bootPageDirectoryPhyaddr], cr3" : [bootPageDirectoryPhyaddr]"=r"(bootPageDirectoryPhyaddr) : : "memory");
    PageDirectory* bootPageDirectory = reinterpret_cast<PageDirectory*>(physToVirt(bootPageDirectoryPhyaddr));

    ASSERT(!current);
    current = rt::makeShared<MemoryMapping>(bootPageDirectory);
  }


  void MemoryMapping::makeCurrent(rt::SharedPtr<MemoryMapping> memoryMapping)
  {
    asm volatile ("mov cr3, %[pageDirectoryPhyaddr]" : : [pageDirectoryPhyaddr]"r"(virtToPhys(reinterpret_cast<uintptr_t>(memoryMapping->m_pageDirectory))) : "memory");
    current = rt::move(memoryMapping);
  }

  rt::SharedPtr<MemoryMapping> MemoryMapping::allocate()
  {
    void* page = allocPages(1);
    if(!page)
      return nullptr;

    ASSERT(current);
    const auto& currentPageDirectory = *current->m_pageDirectory;
    auto& pageDirectory = *static_cast<PageDirectory*>(page);

    rt::fill(rt::begin(pageDirectory), rt::end(pageDirectory), PageDirectoryEntry());
    rt::copy(&currentPageDirectory[768], &currentPageDirectory[1024], &pageDirectory[768]);

    return rt::makeShared<MemoryMapping>(&pageDirectory);
  }

  rt::SharedPtr<MemoryMapping> MemoryMapping::clone()
  {
    auto memoryMapping = allocate();
    if(!memoryMapping)
      return nullptr;

    // Remap ourselves write only
    for(auto& memoryArea : m_memoryAreas)
      mapReadonly(memoryArea);

    memoryMapping->m_memoryAreas = this->m_memoryAreas;
    return memoryMapping;
  }

  MemoryMapping::MemoryMapping(PageDirectory* pageDirectory) : m_pageDirectory(pageDirectory) {}
  MemoryMapping::~MemoryMapping()
  {
    for(auto& memoryArea : m_memoryAreas)
      unmap(memoryArea);

    freePages(m_pageDirectory, 1);
  }

  /*
   * Public interface
   */
  namespace
  {
    constexpr size_t roundDown(size_t index, size_t alignment) { return index / alignment * alignment; }
    constexpr size_t roundUp(size_t index, size_t alignment) { return roundDown(index + alignment - 1, alignment); }
  }

  Result<void> MemoryMapping::map(uintptr_t addr, size_t length, Prot prot, rt::SharedPtr<vfs::File> file, size_t offset)
  {
    uintptr_t begin = roundDown(addr, PAGE_SIZE);
    uintptr_t end   = roundUp(addr+length, PAGE_SIZE);

    if(rt::any(m_memoryAreas.begin(), m_memoryAreas.end(), [&](const auto& memoryArea) { return memoryArea.addr+memoryArea.length>begin && memoryArea.addr < end; }))
      return ErrorCode::EXIST;

    auto memoryArea = MemoryArea(begin, end-begin, prot, rt::move(file), offset, MapType::PRIVATE);
    m_memoryAreas.insert(m_memoryAreas.end(), rt::move(memoryArea));
    return {};
  }

  Result<void> MemoryMapping::unmap(uintptr_t addr, size_t length)
  {
    uintptr_t begin = roundDown(addr, PAGE_SIZE);
    uintptr_t end   = roundUp(addr+length, PAGE_SIZE);

    auto it = rt::find_if(m_memoryAreas.begin(), m_memoryAreas.end(), [&](const auto& memoryArea) { return memoryArea.addr == begin && memoryArea.length == end-begin; });
    if(it == m_memoryAreas.end())
      return ErrorCode::NOT_EXIST;
    /* Question: Should we support partial unmap */

    auto& memoryArea = *it;
    unmap(memoryArea);

    m_memoryAreas.remove(it);
    return {};
  }

  Result<void> MemoryMapping::remap(uintptr_t addr, size_t length, size_t newLength)
  {
    uintptr_t begin  = roundDown(addr, PAGE_SIZE);
    uintptr_t end    = roundUp(addr+length, PAGE_SIZE);
    uintptr_t newEnd = roundUp(addr+newLength, PAGE_SIZE);

    if(newEnd == begin)
      return ErrorCode::INVALID;

    auto it = rt::find_if(m_memoryAreas.begin(), m_memoryAreas.end(), [&](const auto& memoryArea) { return memoryArea.addr == begin && memoryArea.length == end-begin; });
    if(it == m_memoryAreas.end())
      return ErrorCode::NOT_EXIST;

    auto& memoryArea = *it;
    remap(memoryArea, newEnd - begin);
    return {};
  }


  /*
   * Private interface
   *
   * These functions never fail, but instead leave partial mapping on out of
   * memory situation, which may be filled in on page fault should memory
   * becomes available at that time, which is why more fatal error need to be
   * detected beforehand.
   * */
  void MemoryMapping::mapReadonly(MemoryArea& memoryArea)
  {
    for(size_t addr=memoryArea.addr; addr!=memoryArea.addr+memoryArea.length; addr+=PAGE_SIZE)
      mapReadonlySingle(memoryArea, addr);
  }

  void MemoryMapping::mapWritable(MemoryArea& memoryArea)
  {
    for(size_t addr=memoryArea.addr; addr!=memoryArea.addr+memoryArea.length; addr+=PAGE_SIZE)
      mapWritableSingle(memoryArea, addr);
  }

  void MemoryMapping::unmap(MemoryArea& memoryArea)
  {
    for(size_t addr=memoryArea.addr; addr!=memoryArea.addr+memoryArea.length; addr+=PAGE_SIZE)
      unmapSingle(memoryArea, addr);
  }

  void MemoryMapping::remap(MemoryArea& memoryArea, size_t newLength)
  {
    size_t oldLength = memoryArea.length;
    memoryArea.length = newLength;
    if(oldLength < newLength)
    {
      // expand the mapping
      for(size_t addr=memoryArea.addr+oldLength; addr!=memoryArea.addr+newLength; addr+=PAGE_SIZE)
        mapReadonlySingle(memoryArea, addr);
    }
    else if(oldLength > newLength)
    {
      // Shrink the mapping
      for(size_t addr=memoryArea.addr+newLength; addr!=memoryArea.addr+oldLength; addr+=PAGE_SIZE)
        unmapSingle(memoryArea, addr);
    }
  }


  /*
   * Internal function
   */
  namespace
  {
    Result<physaddr_t> allocPageTable()
    {
      void* page = allocPages(1);
      if(!page)
        return ErrorCode::OUT_OF_MEMORY;

      auto& pageTable = *static_cast<PageTable*>(page);
      rt::fill(rt::begin(pageTable), rt::end(pageTable), PageTableEntry());
      return virtToPhys(reinterpret_cast<uintptr_t>(page));
    }

    void freePageTable(physaddr_t addr)
    {
      freePages(reinterpret_cast<void*>(physToVirt(addr)), 1);
    }
  }

  void MemoryMapping::mapReadonlySingle(MemoryArea& memoryArea, uintptr_t addr)
  {
    // Page Table
    auto pageDirectoryIndex = (addr / LARGE_PAGE_SIZE) % 1024;
    auto& pageDirectory = *m_pageDirectory;
    auto& pageDirectoryEntry = pageDirectory[pageDirectoryIndex];
    if(!pageDirectoryEntry.present())
    {
      auto phyaddr = allocPageTable();
      if(!phyaddr)
        return;

      pageDirectoryEntry = PageDirectoryEntry(*phyaddr, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::ALL, Permission::READ_WRITE);
    }

    // Page Frame
    size_t pageTableIndex = (addr / PAGE_SIZE) % 1024;
    auto& pageTable       = *reinterpret_cast<PageTable*>(physToVirt(pageDirectoryEntry.address()));
    auto& pageTableEntry  = pageTable[pageTableIndex];

    auto phyaddr = memoryArea.getPageFrame(addr);
    if(!phyaddr)
      return;

    pageTableEntry = PageTableEntry(*phyaddr, TLBMode::LOCAL, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::ALL, Permission::READ_ONLY);
    asm volatile ( "invlpg [%[addr]]" : : [addr]"r"(addr) : "memory");
  }

  void MemoryMapping::mapWritableSingle(MemoryArea& memoryArea, uintptr_t addr)
  {
    // Page Table
    auto pageDirectoryIndex = (addr / LARGE_PAGE_SIZE) % 1024;
    auto& pageDirectory = *m_pageDirectory;
    auto& pageDirectoryEntry = pageDirectory[pageDirectoryIndex];
    ASSERT(pageDirectoryEntry.present());

    // Page Frame
    size_t pageTableIndex = (addr / PAGE_SIZE) % 1024;
    auto& pageTable       = *reinterpret_cast<PageTable*>(physToVirt(pageDirectoryEntry.address()));
    auto& pageTableEntry  = pageTable[pageTableIndex];
    ASSERT(pageTableEntry.present());

    auto phyaddr = memoryArea.getWritablePageFrame(addr);
    if(!phyaddr)
      return;

    pageTableEntry = PageTableEntry(*phyaddr, TLBMode::LOCAL, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::ALL, Permission::READ_WRITE);
    asm volatile ( "invlpg [%[addr]]" : : [addr]"r"(addr) : "memory");
  }

  void MemoryMapping::unmapSingle(MemoryArea& memoryArea, uintptr_t addr)
  {
    auto pageDirectoryIndex = (addr / LARGE_PAGE_SIZE) % 1024;
    auto& pageDirectory = *m_pageDirectory;
    auto& pageDirectoryEntry = pageDirectory[pageDirectoryIndex];
    if(!pageDirectoryEntry.present())
      return;

    size_t pageTableIndex = (addr / PAGE_SIZE) % 1024;
    auto& pageTable       = *reinterpret_cast<PageTable*>(physToVirt(pageDirectoryEntry.address()));
    auto& pageTableEntry  = pageTable[pageTableIndex];
    if(!pageTableEntry.present())
      return;

    // Page Frame
    pageTableEntry = PageTableEntry();
    asm volatile ( "invlpg [%[addr]]" : : [addr]"r"(addr) : "memory");
    memoryArea.removePageFrame(addr);

    if(rt::any(rt::begin(pageTable), rt::end(pageTable), [](const PageTableEntry& pageTableEntry){ return pageTableEntry.present(); }))
      return;

    // Page Table
    freePageTable(pageDirectoryEntry.address());
    pageDirectoryEntry = PageDirectoryEntry();
  }

  Result<void> MemoryMapping::handlePageFault(uintptr_t addr, uword_t errorCode)
  {
    auto it = rt::find_if(m_memoryAreas.begin(), m_memoryAreas.end(), [&](const auto& memoryArea) { return memoryArea.addr<=addr && addr<memoryArea.addr+memoryArea.length; });
    if(it == m_memoryAreas.end())
      return ErrorCode::NOT_EXIST;

    auto& memoryArea = *it;
    return handlePageFault(memoryArea, addr, errorCode);
  }

  namespace
  {
    static constexpr uword_t PF_PRESENT        = 1u << 0;
    static constexpr uword_t PF_WRITE          = 1u << 1;
    static constexpr uword_t PF_USER           = 1u << 2;
    static constexpr uword_t PF_RESERVED_WRITE = 1u << 3;
    static constexpr uword_t PF_IFETCH         = 1u << 4;
  }
  Result<void> MemoryMapping::handlePageFault(MemoryArea& memoryArea, size_t addr, uword_t errorCode)
  {
    addr = addr / PAGE_SIZE * PAGE_SIZE;
    if(errorCode & PF_USER)
    {
      if(errorCode & PF_PRESENT)
      {
        if(errorCode & PF_IFETCH)
          return ErrorCode::FAULT;

        if((errorCode & PF_WRITE) && (memoryArea.prot & Prot::WRITE) != Prot::WRITE)
        {
          rt::log("Write protection violation\n");
          return ErrorCode::FAULT;
        }

        mapWritableSingle(memoryArea, addr);
      }
      else
        mapReadonlySingle(memoryArea, addr);
    }
    else
      ASSERT_UNREACHABLE;

    return {};
  }
}


#include <i686/memory/MemoryMapping.hpp>

#include <generic/BootInformation.hpp>
#include <i686/interrupts/Interrupts.hpp>
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

  void pageFaultHandler(uint8_t irqNumber, uint32_t errorCode, uintptr_t oldEip)
  {
    uint32_t address;
    asm volatile ("mov %[address], cr2" : [address]"=rm"(address) : :);
    rt::logf("\nPage Fault at 0x%lx with error code 0x%lx and old eip 0x%lx\n", address, errorCode, oldEip);
    rt::panic("Page Fault\n");
  }


  void MemoryMapping::initialize()
  {
    interrupts::uninstallHandler(14);
    interrupts::installHandler(14, &pageFaultHandler, PrivilegeLevel::RING0, true);

    physaddr_t bootPageDirectoryPhyaddr;
    asm volatile ("mov %[bootPageDirectoryPhyaddr], cr3" : [bootPageDirectoryPhyaddr]"=r"(bootPageDirectoryPhyaddr) : : "memory");
    PageDirectory* bootPageDirectory = reinterpret_cast<PageDirectory*>(physToVirt(bootPageDirectoryPhyaddr));
    currentMemoryMapping = rt::makeShared<MemoryMapping>(bootPageDirectory);
    currentMemoryMapping->unmap(0, 0);
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
    void* page = allocPages(1);
    if(!page)
      return nullptr;

    const auto& currentPageDirectory = *current()->m_pageDirectory;
    auto& pageDirectory = *static_cast<PageDirectory*>(page);

    rt::fill(rt::begin(pageDirectory), rt::end(pageDirectory), PageDirectoryEntry());
    rt::copy(&currentPageDirectory[768], &currentPageDirectory[1024], &pageDirectory[768]);

    return rt::makeShared<MemoryMapping>(&pageDirectory);
  }

  MemoryMapping::MemoryMapping(PageDirectory* pageDirectory) : m_pageDirectory(pageDirectory) {}
  MemoryMapping::~MemoryMapping()
  {
    for(auto& memoryArea : m_memoryAreas)
      unmap(*memoryArea);

    freePages(m_pageDirectory, 1);
  }

  namespace
  {
    constexpr size_t roundDown(size_t index, size_t alignment) { return index / alignment * alignment; }
    constexpr size_t roundUp(size_t index, size_t alignment) { return roundDown(index + alignment - 1, alignment); }
    Result<physaddr_t> allocPageTable()
    {
      void* page = allocPages(1);
      if(!page)
        return ErrorCode::OUT_OF_MEMORY;


      auto& pageTable = *static_cast<PageTable*>(page);
      rt::fill(rt::begin(pageTable), rt::end(pageTable), PageTableEntry());
      return virtToPhys(reinterpret_cast<uintptr_t>(page));
    }

    Result<physaddr_t> allocPageFrame(rt::SharedPtr<vfs::File> file, size_t offset)
    {
      void* page = allocPages(1);
      if(!page)
        return ErrorCode::OUT_OF_MEMORY;

      auto& pageFrame = *static_cast<PageFrame*>(page);
      rt::fill(rt::begin(pageFrame), rt::end(pageFrame), '\0');
      if(file)
      {
        file->seek(Anchor::BEGIN, offset);
        file->read(pageFrame, sizeof pageFrame); // We do not care, if it failed, we are left with an empty page
      }

      return virtToPhys(reinterpret_cast<uintptr_t>(page));
    }

    void freePageTable(physaddr_t addr)
    {
      freePages(reinterpret_cast<void*>(physToVirt(addr)), 1);
    }

    void freePageFrame(physaddr_t addr)
    {
      freePages(reinterpret_cast<void*>(physToVirt(addr)), 1);
    }
  }

  Result<void> MemoryMapping::map(uintptr_t addr, size_t length, Prot prot, rt::SharedPtr<vfs::File> file, size_t offset)
  {
    uintptr_t begin = roundDown(addr, PAGE_SIZE);
    uintptr_t end   = roundUp(addr+length, PAGE_SIZE);

    if(rt::any(m_memoryAreas.begin(), m_memoryAreas.end(), [&](const rt::SharedPtr<MemoryArea>& memoryArea) { return memoryArea->addr+memoryArea->length>begin && memoryArea->addr < end; }))
      return ErrorCode::EXIST;

    auto memoryArea = rt::makeShared<MemoryArea>(begin, end-begin, prot, rt::move(file), offset, MemoryArea::Type::PRIVATE);
    if(!memoryArea)
      return ErrorCode::OUT_OF_MEMORY;

    m_memoryAreas.insert(m_memoryAreas.end(), memoryArea);

    map(*memoryArea);
    return {};
  }

  /* These two functions never fail, but instead leave partial mapping on out of
   * memory situation, which may be filled in on page fault should memory
   * becomes available at that time, which is why more fatal error need to be
   * detected beforehand. */
  void MemoryMapping::map(MemoryArea& memoryArea)
  {
    for(size_t addr=memoryArea.addr; addr!=memoryArea.addr+memoryArea.length; addr+=PAGE_SIZE)
    {
      size_t offset = memoryArea.offset+(addr-memoryArea.addr);
      mapSingle(memoryArea, addr, offset);
    }
  }

  void MemoryMapping::mapSingle(MemoryArea& memoryArea, uintptr_t addr, size_t offset)
  {
    auto permission = (memoryArea.prot & Prot::WRITE) == Prot::WRITE ? Permission::READ_WRITE : Permission::READ_ONLY;
    //auto permission = Permission::READ_WRITE;

    auto pageDirectoryIndex = (addr / LARGE_PAGE_SIZE) % 1024;
    auto& pageDirectory = *m_pageDirectory;
    auto& pageDirectoryEntry = pageDirectory[pageDirectoryIndex];
    if(!pageDirectoryEntry.present())
    {
      auto result = allocPageTable();
      if(!result)
        return;

      pageDirectoryEntry = PageDirectoryEntry(*result, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::ALL, Permission::READ_WRITE);
    }
    else
    {
      /* This means we are sharing the page directory with another mapping,
       * which is perfectly fine. DO NOTHING */
    }

    size_t pageTableIndex = (addr / PAGE_SIZE) % 1024;
    auto& pageTable       = *reinterpret_cast<PageTable*>(physToVirt(pageDirectoryEntry.address()));
    auto& pageTableEntry  = pageTable[pageTableIndex];
    if(!pageTableEntry.present())
    {
      auto result = allocPageFrame(memoryArea.file, offset);
      if(!result)
        return;

      pageTableEntry = PageTableEntry(*result, TLBMode::LOCAL, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::ALL, permission);
      asm volatile ( "invlpg [%[addr]]" : : [addr]"r"(addr) : "memory");
    }
    else
    {
      /* This means we are sharing the page table with another mapping, which is
       * impossible since this is the smallest unit of allocation. Mayday. */
      ASSERT_UNREACHABLE;
    }
  }

  Result<void> MemoryMapping::unmap(uintptr_t addr, size_t length)
  {
    uintptr_t begin = roundDown(addr, PAGE_SIZE);
    uintptr_t end   = roundUp(addr+length, PAGE_SIZE);

    auto it = rt::find_if(m_memoryAreas.begin(), m_memoryAreas.end(), [&](const rt::SharedPtr<MemoryArea>& memoryArea) { return memoryArea->addr == begin && memoryArea->length == end-begin; });
    if(it == m_memoryAreas.end())
      return ErrorCode::NOT_EXIST;
    /* Question: Should we support partial unmap */

    auto memoryArea = *it;
    m_memoryAreas.erase(it);

    unmap(*memoryArea);
    return {};
  }

  void MemoryMapping::unmap(MemoryArea& memoryArea)
  {
    for(size_t addr=memoryArea.addr; addr!=memoryArea.addr+memoryArea.length; addr+=PAGE_SIZE)
      unmapSingle(memoryArea, addr);
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

    if(true)
    {
      freePageFrame(pageTableEntry.address());
      pageTableEntry = PageTableEntry();
      asm volatile ( "invlpg [%[addr]]" : : [addr]"r"(addr) : "memory");
    }

    if(rt::none(rt::begin(pageTable), rt::end(pageTable), [](const PageTableEntry& pageTableEntry){ return pageTableEntry.present(); }))
    {
      freePageTable(pageDirectoryEntry.address());
      pageDirectoryEntry = PageDirectoryEntry();
    }
  }

  Result<void> MemoryMapping::remap(uintptr_t addr, size_t length, size_t newLength)
  {
    uintptr_t begin  = roundDown(addr, PAGE_SIZE);
    uintptr_t end    = roundUp(addr+length, PAGE_SIZE);
    uintptr_t newEnd = roundUp(addr+newLength, PAGE_SIZE);

    if(newEnd == begin)
      return ErrorCode::INVALID;

    auto it = rt::find_if(m_memoryAreas.begin(), m_memoryAreas.end(), [&](const rt::SharedPtr<MemoryArea>& memoryArea) { return memoryArea->addr == begin && memoryArea->length == end-begin; });
    if(it == m_memoryAreas.end())
      return ErrorCode::NOT_EXIST;

    auto memoryArea = *it;
    remap(*memoryArea, newEnd - begin);
    return {};
  }

  void MemoryMapping::remap(MemoryArea& memoryArea, size_t newLength)
  {

    size_t oldLength = memoryArea.length;
    memoryArea.length = newLength;
    if(oldLength < newLength)
    {
      // expand the mapping
      for(size_t addr=memoryArea.addr+oldLength; addr!=memoryArea.addr+newLength; addr+=PAGE_SIZE)
      {
        size_t offset = memoryArea.offset+(addr-memoryArea.addr);
        mapSingle(memoryArea, addr, offset);
      }
    }
    else if(oldLength > newLength)
    {
      // Shrink the mapping
      for(size_t addr=memoryArea.addr+newLength; addr!=memoryArea.addr+oldLength; addr+=PAGE_SIZE)
        unmapSingle(memoryArea, addr);
    }
  }

}


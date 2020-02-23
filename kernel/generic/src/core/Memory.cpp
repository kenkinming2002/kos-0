/**
 * @file generic/src/core/memory/Memory.cpp
 *
 * This file contains hooks to liballoc library which serve as an implementation
 * for kernel memory allocation
 *
 * This is resposible for initialization of kernel memory subsystem
 */
#include <generic/core/Memory.hpp>

#include <i686/core/memory/MemoryMapping.hpp>

#include <i686/boot/boot.hpp>
#include <generic/io/Print.hpp>

#include <i686/core/Interrupt.hpp>

#include <cstddef>

#include <liballoc_1_1.h>

namespace core::memory
{
  /******************************************
   * Initialization and Page Fault Handling *
   ******************************************/
  namespace
  {
    [[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void page_fault_handler(core::interrupt::frame*, core::interrupt::uword_t error)
    {
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
    } }

  void init()
  {
    core::interrupt::install_handler(0xE, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&page_fault_handler));
  }

  /***********
   * Globals *
   ***********/
#define MEMROY_INIT_PRIORITY __attribute__((init_priority(65535)))
  namespace
  {
    bool pageFrameAllocatorInitialized = false;
    MEMROY_INIT_PRIORITY memory::PhysicalMemoryRegionAllocator<memory::LinkedListMemoryRegionAllocator> physicalMemoryRegionAllocator
    (
        utils::deref_cast<BootInformation>(bootInformationStorage).memoryMapEntries, 
        utils::deref_cast<BootInformation>(bootInformationStorage).memoryMapEntriesCount
    );
    MEMROY_INIT_PRIORITY memory::VirtualMemoryRegionAllocator<memory::LinkedListMemoryRegionAllocator> virtualMemoryRegionAllocator(reinterpret_cast<std::byte*>(0xD0000000), reinterpret_cast<std::byte*>(0xE000000));
    MEMROY_INIT_PRIORITY memory::PageFrameAllocator pageFrameAllocator(physicalMemoryRegionAllocator, virtualMemoryRegionAllocator);
  }
#undef MEMROY_INIT_PRIORITY

  /*************
   * Interface *
   *************/
  void* malloc(size_t size)
  { return kmalloc(size); }

  void free(void* ptr)
  {
    kfree(ptr);
  }

  std::pair<void*, phyaddr_t> mallocMappedPages(size_t count)
  {
    return pageFrameAllocator.allocate(count);
  }

  void freeMappedPages(void* pages, size_t count)
  {
    pageFrameAllocator.deallocate(pages, count);
  }

  std::optional<MemoryRegion> mallocPhysicalPages(size_t count)
  {
    return physicalMemoryRegionAllocator.allocate(count);
  }
  
  void freePhysicalPages(MemoryRegion physicalMemoryRegion)
  {
    physicalMemoryRegionAllocator.deallocate(physicalMemoryRegion);
  }
}

/**********************
 * Hooks for liballoc *
 **********************/
__attribute__((aligned(4096)))std::byte early_heap[0x10000];

extern "C" int liballoc_lock()   { return 0; }
extern "C" int liballoc_unlock() { return 0; }

extern "C" void* liballoc_alloc(size_t n)
{
  static bool late_allocation = false;
  if(!late_allocation)
  {
    late_allocation = true;
    return early_heap;
  }
  else
  {
    if(!core::memory::pageFrameAllocatorInitialized)
    {
      io::print("WHAT THE DUCK");
      asm("hlt");
    }
    return core::memory::pageFrameAllocator.allocate(n).first;
  }
}

extern "C" int liballoc_free(void* pages, size_t n)
{
  core::memory::pageFrameAllocator.deallocate(pages, n);
  return 0;
}

void* operator new (std::size_t size)
{
  return kmalloc(size);
}

void operator delete(void* ptr) noexcept
{
  kfree(ptr);
}

void operator delete(void* ptr, unsigned long) noexcept
{
  kfree(ptr);
}

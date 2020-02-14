/**
 * @file generic/src/core/memory/Memory.cpp
 *
 * This file contains hooks to liballoc library which serve as an implementation
 * for kernel memory allocation
 *
 * This is resposible for initialization of kernel memory subsystem
 */
#include <generic/core/Memory.hpp>

#include <i686/boot/lower_half.hpp>
#include <generic/io/Print.hpp>

#include <cstddef>

#include <liballoc_1_1.h>

extern "C"
{
  extern std::byte early_heap_start[];
  extern std::byte early_heap_end[];
}

//namespace core
//{
//  //core::memory::PageFrameAllocator* pageFrameAllocator;
//}

namespace
{
  bool pageFrameAllocatorInitialized = false;
}

namespace core
{
  Memory::Memory() 
    : m_physicalPageFrameAllocator(bootInformation.mmap_entries, bootInformation.mmap_entries_count),
      m_virtualPageFrameAllocator(reinterpret_cast<std::byte*>(0xD0000000), reinterpret_cast<std::byte*>(0xE000000)),
      m_pageFrameAllocator(m_physicalPageFrameAllocator, m_virtualPageFrameAllocator)
  {
    pageFrameAllocatorInitialized = true;
  }

  void* Memory::allocate(size_t n)
  {
    return m_pageFrameAllocator.allocate(n);
  }

  void Memory::deallocate(void* pages, size_t n)
  {
    m_pageFrameAllocator.deallocate(reinterpret_cast<core::memory::PageFrame<>*>(pages), n);
  }

  __attribute__((init_priority(65535))) Memory gMemory;
}

extern "C" int liballoc_lock()   { return 0; }
extern "C" int liballoc_unlock() { return 0; }

extern "C" void* liballoc_alloc(size_t n)
{
  static bool late_allocation = false;
  if(!late_allocation)
  {
    late_allocation = true;
    return early_heap_start;
  }
  else
  {
    if(!pageFrameAllocatorInitialized)
    {
      io::print("WHAT THE DUCK");
      asm("hlt");
    }
    return core::gMemory.allocate(n);
  }
}

extern "C" int liballoc_free(void* pages, size_t n)
{
  core::gMemory.deallocate(pages, n);
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

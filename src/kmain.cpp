#include <io/Serial.hpp>

#include <grub/multiboot2.h>
#include <boot/lower_half.hpp>

#include <core/Interrupt.hpp>
#include <core/Segmentation.hpp>
#include <core/Paging.hpp>

#include <iterator>

#include <core/memory/PhysicalPageFrameAllocator.hpp>
#include <core/memory/VirtualPageFrameAllocator.hpp>
#include <core/memory/PageFrameAllocator.hpp>

#include <generic/core/Memory.hpp>

#include <intel/core/pic/8259.hpp>

#include <io/Framebuffer.hpp>

#include <utils/Format.hpp>

#include <io/Print.hpp>

#include <utility>

const char str[] = "Hello World!";

#define virtual_address(addr) (void*)((char*)addr+0xC0000000)

extern "C" void abort()
{
  asm("hlt");
  __builtin_unreachable();
}

void handler()
{
}

/**
 * Log structures available after entering kmain()
 */
void startup_log()
{
  // Framebuffer
  io::print("FRAMEBUFFER:", 
    ",  addr:", (uint64_t)bootInformation.framebuffer.common.framebuffer_addr,
    ",  width:", (uint32_t)bootInformation.framebuffer.common.framebuffer_width,
    ",  height:", (uint32_t)bootInformation.framebuffer.common.framebuffer_height);

  io::print("MEMORY MAP---", " version:", (uint16_t)bootInformation.mmap.entry_version, "  entries:");
  for(size_t i=0 ; i<bootInformation.mmap_entries_count; ++i)
    io::print( "  type:", (uint32_t)bootInformation.mmap_entries[i].type, ", addr:", (uint64_t)bootInformation.mmap_entries[i].addr, ", len:", (uint64_t)bootInformation.mmap_entries[i].len);
}

void test_physicalPageFrameAllocator(core::memory::PhysicalPageFrameAllocator& physicalPageFrameAllocator)
{
  core::memory::PhysicalPageFrameRange chunks[10];
  for(size_t i=0; i<10; ++i)
  {
    auto physicalPageFrameRange = physicalPageFrameAllocator.allocate(i+1);
    if(physicalPageFrameRange)
    {
      chunks[i] = physicalPageFrameRange.value();
    }
    else
      io::print("Failed");
  }

  for(size_t i=0; i<10; ++i)
    physicalPageFrameAllocator.deallocate(chunks[i]);

  for(size_t i=0; i<10; ++i)
  {
    auto physicalPageFrameRange = physicalPageFrameAllocator.allocate(i+1);
    if(physicalPageFrameRange)
    {
      if(chunks[i].index != physicalPageFrameRange->index)
        // Reallocation should yield same physical memory chunk as merging is implemented and all chunk allocated is freed
        io::print("Discrepancy at ", i); 
      chunks[i] = *physicalPageFrameRange;
    }
    else
      io::print("Failed");
  }

  for(size_t i=0; i<10; ++i)
    physicalPageFrameAllocator.deallocate(chunks[i]);
}

void test_virtualPageFrameAllocator(core::memory::VirtualPageFrameAllocator& virtualPageFrameAllocator)
{
  auto virtualPageFrameRange = virtualPageFrameAllocator.allocate(1);
  if(virtualPageFrameRange)
    io::print("Virtual Page Frame - Index:", virtualPageFrameRange->index, ", count:", virtualPageFrameRange->count);
  else
      io::print("Virtual Page Frame Allocator - Failed");
}

void test_pageFrameAllocation(core::memory::PageFrameAllocator& pageFrameAllocator)
{
  auto pageFrames = pageFrameAllocator.allocate(1);
  if(!pageFrames)
    io::print("Page Frame Allocator - Failed");

  for(size_t i=0; i<std::decay_t<decltype(*pageFrames)>::SIZE; ++i)
    pageFrames->data[i] = i % 128;

  for(size_t i=0; i<std::decay_t<decltype(*pageFrames)>::SIZE; ++i)
    if(pageFrames->data[i] != (i % 128))
      io::print("Page Frame Allocator - Failed");
}

extern "C" int kmain()
{
  /** Old-Styled Initialization **/
  serial_configure(SERIAL_COM1_BASE, 1);
  //serial_write(SERIAL_COM1_BASE, str, 11);
  //

  /** *Global* data**/
  core::Interrupt interrupt;
  core::pic::Controller8259 controller8259;

  /** Initialize **/
  utils::Callback callback(&handler);
  for(int i=0; i<256; ++i)
    interrupt.installHandler(i, core::PrivillegeLevel::RING0, callback);

  io::framebuffer::init();
  /** Logging **/
  //startup_log();

  for(int i=0; i<10000; ++i)
  {
    void* mem = kmalloc(40);
    io::print("kmain-malloc ", reinterpret_cast<uintptr_t>(mem));
  }

  for(int i=0; i<10000; ++i)
  {
    void* mem = kmalloc(4);
    io::print("kmain-malloc ", reinterpret_cast<uintptr_t>(mem));
    kfree(mem);
  }
  
  /** Testing **/
  asm("int $20");

  return 0;
}

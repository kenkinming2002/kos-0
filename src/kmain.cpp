#include <io/serial.h>

#include <grub/multiboot2.h>
#include <boot/lower_half.hpp>

#include <core/Interrupt.hpp>
#include <core/Segmentation.hpp>
#include <core/Paging.hpp>

#include <iterator>

#include <core/memory/PhysicalPageFrameAllocator.hpp>
#include <core/memory/VirtualPageFrameAllocator.hpp>
#include <core/memory/PageFrameAllocator.hpp>

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

void test_physicalPageFrameAllocator(core::memory::StaticPhysicalPageFrameAllocator& staticPhysicalPageFrameAllocator)
{
  core::memory::PhysicalPageFrameRange chunks[10];
  for(size_t i=0; i<10; ++i)
  {
    auto physicalPageFrameRange = staticPhysicalPageFrameAllocator.allocate(i+1);
    if(physicalPageFrameRange)
    {
      chunks[i] = physicalPageFrameRange.value();
    }
    else
      io::print("Failed");
  }

  for(size_t i=0; i<10; ++i)
    staticPhysicalPageFrameAllocator.deallocate(chunks[i]);

  for(size_t i=0; i<10; ++i)
  {
    auto physicalPageFrameRange = staticPhysicalPageFrameAllocator.allocate(i+1);
    if(physicalPageFrameRange)
    {
      if(chunks[i].index != physicalPageFrameRange->index)
        io::print("Discrepancy at ", i); // Reallocation should yield same physical memory chunk as merging is implemented and all chunk allocated is freed
      chunks[i] = *physicalPageFrameRange;
    }
    else
      io::print("Failed");
  }

  for(size_t i=0; i<10; ++i)
    staticPhysicalPageFrameAllocator.deallocate(chunks[i]);
}

void test_virtualPageFrameAllocator(core::memory::VirtualPageFrameAllocator& virtualPageFrameAllocator)
{
  auto virtualPageFrameRange = virtualPageFrameAllocator.getUsableVirtualPageFrameRange(1);
  if(virtualPageFrameRange)
    io::print("Virtual Page Frame - Index:", virtualPageFrameRange->index, ", count:", virtualPageFrameRange->count);
  else
      io::print("Virtual Page Frame Allocator - Failed");
}

void test_pageFrameAllocation(core::memory::PageFrameAllocator<core::memory::StaticPhysicalPageFrameAllocator, core::memory::VirtualPageFrameAllocator>& pageFrameAllocator)
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
  //serial_configure(SERIAL_COM1_BASE, 1);
  //serial_write(SERIAL_COM1_BASE, str, 11);
  //

  /** *Global* data**/
  core::Interrupt interrupt;
  core::Segmentation segmentation;
  core::pic::Controller8259 controller8259;
  core::memory::StaticPhysicalPageFrameAllocator staticPhysicalPageFrameAllocator(bootInformation.mmap_entries, bootInformation.mmap_entries_count);
  core::memory::VirtualPageFrameAllocator virtualPageFrameAllocator;

  /** Initialize **/
  utils::Callback callback(&handler);
  for(int i=0; i<256; ++i)
    interrupt.installHandler(i, core::PrivillegeLevel::RING0, callback);
  io::framebuffer::init();
  /** Logging **/
  //startup_log();
  test_physicalPageFrameAllocator(staticPhysicalPageFrameAllocator);
  test_virtualPageFrameAllocator(virtualPageFrameAllocator);

  core::memory::PageFrameAllocator pageFrameAllocator(std::move(staticPhysicalPageFrameAllocator), std::move(virtualPageFrameAllocator));

  test_pageFrameAllocation(pageFrameAllocator);
  
  /** Testing **/
  asm("int $20");

  return 0;
}

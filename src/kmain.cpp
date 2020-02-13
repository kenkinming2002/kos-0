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
#include <intel/asm/io.hpp>

#include <io/Framebuffer.hpp>

#include <utils/Format.hpp>

#include <io/Print.hpp>

#include <utility>

#include <liballoc_1_1.h>

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

[[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void null_interrupt_handler([[maybe_unused]]core::interrupt::frame* frame)
{
  io::print("Interrupt START");
}

[[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void general_pretection_fault_handler([[maybe_unused]]core::interrupt::frame* frame, core::interrupt::uword_t error_code)
{
  io::print("GPF START ", (uint32_t)error_code); 
  asm("hlt");
}

[[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void keyboard_interrupt_handler([[maybe_unused]]core::interrupt::frame* frame)
{
  io::print("KEYBOARD INTERRUPT");

  uint8_t scanCode = assembly::inb(0x60);
  io::print("Scan code:", (uint16_t)scanCode);

  core::pic::controller8259::acknowledge(0x1);
  io::print("KEYBOARD INTERRUPT acknowledged");
}

[[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void timer_interrupt_handler([[maybe_unused]]core::interrupt::frame* frame)
{
  io::print("TIMER INTERRUPT");
  core::pic::controller8259::acknowledge(0x0);
  io::print("TIMER INTERRUPT acknowledged");
}

extern "C" int kmain()
{
  /** Old-Styled Initialization **/
  //serial_configure(SERIAL_COM1_BASE, 1);
  //serial_write(SERIAL_COM1_BASE, str, 11);
  
  /** *Global* data**/
  core::interrupt::init();
  core::pic::controller8259::init();

  core::interrupt::install_handler(0x0D, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&general_pretection_fault_handler));
  core::interrupt::install_handler(0x20, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&timer_interrupt_handler));
  core::interrupt::install_handler(0x21, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&keyboard_interrupt_handler));
  core::interrupt::install_handler(0x22, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&null_interrupt_handler));

  core::pic::controller8259::clearMask(0); // Enable timer
  core::pic::controller8259::clearMask(1); // Enable keyboard

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
  asm("int $0x22");
  asm("int $0x22");

  asm("hlt");

  return 0;
}

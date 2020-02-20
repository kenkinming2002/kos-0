#include <generic/io/Serial.hpp>

#include <generic/grub/multiboot2.h>
#include <i686/boot/boot.hpp>

#include <i686/core/Interrupt.hpp>

#include <iterator>

#include <generic/core/memory/PhysicalPageFrameAllocator.hpp>
#include <generic/core/memory/VirtualPageFrameAllocator.hpp>
#include <generic/core/memory/PageFrameAllocator.hpp>

#include <generic/core/Memory.hpp>

#include <intel/core/pic/8259.hpp>
#include <intel/asm/io.hpp>

#include <generic/io/Framebuffer.hpp>
#include <generic/io/PS2Keyboard.hpp>

#include <generic/io/Print.hpp>

#include <utility>

#include <liballoc_1_1.h>

const char str[] = "Hello World!";

#define virtual_address(addr) (void*)((char*)addr+0xC0000000)

extern "C" void abort()
{
  for(;;)
    asm("hlt");
}

void handler()
{
}

/**
 * Log structures available after entering kmain()
 */
//void startup_log()
//{
//  // Framebuffer
//  io::print("FRAMEBUFFER:", 
//    ",  addr:", (uint32_t)bootInformation.framebuffer.common.framebuffer_addr,
//    ",  width:", (uint32_t)bootInformation.framebuffer.common.framebuffer_width,
//    ",  height:", (uint32_t)bootInformation.framebuffer.common.framebuffer_height);
//
//  io::print("MEMORY MAP---", " version:", (uint16_t)bootInformation.mmap.entry_version, "  entries:");
//  for(size_t i=0 ; i<bootInformation.mmap_entries_count; ++i)
//    io::print( "  type:", (uint32_t)bootInformation.mmap_entries[i].type, ", addr:", (uint32_t)bootInformation.mmap_entries[i].addr, ", len:", (uint32_t)bootInformation.mmap_entries[i].len);
//}

//void test_physicalPageFrameAllocator(core::memory::PhysicalPageFrameAllocator& physicalPageFrameAllocator)
//{
//  core::memory::PhysicalPageFrameRange chunks[10];
//  for(size_t i=0; i<10; ++i)
//  {
//    auto physicalPageFrameRange = physicalPageFrameAllocator.allocate(i+1);
//    if(physicalPageFrameRange)
//    {
//      chunks[i] = physicalPageFrameRange.value();
//    }
//    else
//      io::print("Failed");
//  }
//
//  for(size_t i=0; i<10; ++i)
//    physicalPageFrameAllocator.deallocate(chunks[i]);
//
//  for(size_t i=0; i<10; ++i)
//  {
//    auto physicalPageFrameRange = physicalPageFrameAllocator.allocate(i+1);
//    if(physicalPageFrameRange)
//    {
//      if(chunks[i].index != physicalPageFrameRange->index)
//        // Reallocation should yield same physical memory chunk as merging is implemented and all chunk allocated is freed
//        io::print("Discrepancy at ", i); 
//      chunks[i] = *physicalPageFrameRange;
//    }
//    else
//      io::print("Failed");
//  }
//
//  for(size_t i=0; i<10; ++i)
//    physicalPageFrameAllocator.deallocate(chunks[i]);
//}
//
//void test_virtualPageFrameAllocator(core::memory::VirtualPageFrameAllocator& virtualPageFrameAllocator)
//{
//  auto virtualPageFrameRange = virtualPageFrameAllocator.allocate(1);
//  if(virtualPageFrameRange)
//    io::print("Virtual Page Frame - Index:", virtualPageFrameRange->index, ", count:", virtualPageFrameRange->count);
//  else
//      io::print("Virtual Page Frame Allocator - Failed");
//}
//
//void test_pageFrameAllocation(core::memory::PageFrameAllocator& pageFrameAllocator)
//{
//  auto pageFrames = pageFrameAllocator.allocate(1);
//  if(!pageFrames)
//    io::print("Page Frame Allocator - Failed");
//
//  for(size_t i=0; i<std::decay_t<decltype(*pageFrames)>::SIZE; ++i)
//    pageFrames->data[i] = i % 128;
//
//  for(size_t i=0; i<std::decay_t<decltype(*pageFrames)>::SIZE; ++i)
//    if(pageFrames->data[i] != (i % 128))
//      io::print("Page Frame Allocator - Failed");
//}

[[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void null_interrupt_handler([[maybe_unused]]core::interrupt::frame* frame)
{
  io::print("Interrupt START");
}

[[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void timer_interrupt_handler([[maybe_unused]]core::interrupt::frame* frame)
{
  io::print("TIMER INTERRUPT\n");

  io::print("TIMER INTERRUPT acknowledged\n");
  core::pic::controller8259::acknowledge(0x0);
}

extern "C" int kmain()
{
  /** Old-Styled Initialization **/
  //serial_configure(SERIAL_COM1_BASE, 1);
  //serial_write(SERIAL_COM1_BASE, str, 11);
  
  /** *Global* data**/
  core::interrupt::install_handler(0x20, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&timer_interrupt_handler));
  core::interrupt::install_handler(0x22, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&null_interrupt_handler));

  io::print("DEBUG: Modules\n");
  auto& bootInformation = utils::deref_cast<BootInformation>(bootInformationStorage);
  for(auto* moduleEntry = bootInformation.moduleEntries; moduleEntry != nullptr; moduleEntry = moduleEntry->next)
  {
    io::print("addr: ", (uintptr_t)moduleEntry->addr, ", len: ", moduleEntry->len);
    typedef void(*call_module_t)(void);
    call_module_t start_program = (call_module_t)moduleEntry->addr;
    start_program();
  }

  //core::pic::controller8259::clearMask(0); // Enable timer

  /** Logging **/
  //for(int i=0; i<6862; ++i)
  //{
  //  void* mem = kmalloc(40);
  //  io::print("kmain-malloc ", reinterpret_cast<uintptr_t>(mem), "\n");
  //}

  //for(int i=0; i<10000; ++i)
  //{
  //  void* mem = kmalloc(4);
  //  io::print("kmain-malloc with free", reinterpret_cast<uintptr_t>(mem), "\n");
  //  kfree(mem);
  //}
  
  
  for(;;)
  {
    if(auto res = io::ps2Keyboard.poll())
    {
      switch(res->keyState)
      {
      case io::PS2Keyboard::KeyState::PRESSED:
        if(auto c = res->toAscii())
        {
          char str[] = {*c, '\0'};
          io::print(str);
          break;
        }
      case io::PS2Keyboard::KeyState::RELEASED:
        //io::print("Key Released ", static_cast<uint16_t>(res->keyCode));
        break;
      }
    }
  }
  
  /** Testing **/
  asm("int $0x22");
  asm("int $0x22");

  while(true)
    asm("hlt");

  return 0;
}

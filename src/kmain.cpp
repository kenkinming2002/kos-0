#include <generic/io/Serial.hpp>

#include <generic/core/Memory.hpp>
#include <generic/core/Process.hpp>

#include <i686/core/Interrupt.hpp>
#include <i686/core/Syscall.hpp>
#include <i686/core/MultiProcessing.hpp>
#include <intel/core/pic/8259.hpp>

#include <generic/io/PS2Keyboard.hpp>
#include <generic/io/Print.hpp>
#include <generic/io/Framebuffer.hpp>

#include <utility>

const char str[] = "Hello World!";

extern "C" void abort()
{
  for(;;)
    asm("hlt");
}

[[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void null_interrupt_handler([[maybe_unused]]core::interrupt::frame* frame)
{
  CORE_INTERRUPT_ENTRY;

  io::print("Interrupt START");

  CORE_INTERRUPT_EXIT;
}

[[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void timer_interrupt_handler([[maybe_unused]]core::interrupt::frame* frame)
{
  CORE_INTERRUPT_ENTRY;

  io::print("TIMER INTERRUPT\n");

  io::print("TIMER INTERRUPT acknowledged\n");
  core::pic::controller8259::acknowledge(0x0);

  CORE_INTERRUPT_EXIT;
}

int syscall_write(const core::State state)
{
  const char* str = reinterpret_cast<const char*>(state.ebx);
  size_t count = state.esi;
  io::frameBuffer.write(str, count);

  return count;
}

extern "C" int kmain()
{
  /** Old-Styled Initialization **/
  //serial_configure(SERIAL_COM1_BASE, 1);
  //serial_write(SERIAL_COM1_BASE, str, 11);
  
  /** *Global* data**/
  core::interrupt::install_handler(0x20, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&timer_interrupt_handler));
  core::interrupt::install_handler(0x22, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&null_interrupt_handler));

  core::register_syscall_handler(3, &syscall_write);

  io::print("DEBUG: Modules\n");
  auto& bootInformation = utils::deref_cast<BootInformation>(bootInformationStorage);
  for(auto* moduleEntry = bootInformation.moduleEntries; moduleEntry != nullptr; moduleEntry = moduleEntry->next)
  {
    io::print("addr: ", (uintptr_t)moduleEntry->addr, ", len: ", moduleEntry->len, "\n");

    auto* process = new core::Process(0x00000000);;
    process->setAsActive();
    process->addSection(0x00000000, core::memory::Access::ALL, core::memory::Permission::READ_ONLY, 
        reinterpret_cast<const uint8_t*>(moduleEntry->addr), moduleEntry->len);
    core::multiprocessing::processesList.push_front(*process);
  }

  if(!core::multiprocessing::processesList.empty())
    core::multiprocessing::processesList.front().run();
  else
    for(;;) asm("hlt");


  //core::pic::controller8259::clearMask(0); // Enable timer

  /** Logging **/
  //for(int i=0; i<6862; ++i)
  //{
  //  void* mem = core::memory::malloc(40);
  //  io::print("kmain-malloc ", reinterpret_cast<uintptr_t>(mem), "\n");
  //}

  //for(int i=0; i<10000; ++i)
  //{
  //  void* mem = core::memory::malloc(4);
  //  io::print("kmain-malloc with free", reinterpret_cast<uintptr_t>(mem), "\n");
  //  core::memory::free(mem);
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

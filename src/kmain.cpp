#include <generic/io/Serial.hpp>

#include <generic/core/Memory.hpp>
#include <generic/core/Process.hpp>

#include <i686/core/Interrupt.hpp>
#include <i686/core/Syscall.hpp>
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
  io::print("Interrupt START");
}

[[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void timer_interrupt_handler([[maybe_unused]]core::interrupt::frame* frame)
{
  io::print("TIMER INTERRUPT\n");

  io::print("TIMER INTERRUPT acknowledged\n");
  core::pic::controller8259::acknowledge(0x0);
}

core::SyscallResult syscall_write(const core::Command::Operand* operands)
{
  if(reinterpret_cast<uintptr_t>(&operands[2])>0xC0000000)
    return {-1, 0};

  if(operands[0].type != core::Command::Operand::Type::IMMEDIATE)
    return {-1, 0};
  if(operands[1].type != core::Command::Operand::Type::IMMEDIATE)
    return {-1, 0};

  const char* str = reinterpret_cast<const char*>(operands[0].immediate.value);
  size_t count = operands[1].immediate.value;
  io::frameBuffer.write(str, count);

  return {0, 2};
}

extern "C" int kmain()
{
  /** Old-Styled Initialization **/
  //serial_configure(SERIAL_COM1_BASE, 1);
  //serial_write(SERIAL_COM1_BASE, str, 11);
  
  /** *Global* data**/
  core::interrupt::install_handler(0x20, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&timer_interrupt_handler));
  core::interrupt::install_handler(0x22, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&null_interrupt_handler));

  core::register_syscall_handler(core::Command::OpCode::WRITE, &syscall_write);

  io::print("DEBUG: Modules\n");
  auto& bootInformation = utils::deref_cast<BootInformation>(bootInformationStorage);
  for(auto* moduleEntry = bootInformation.moduleEntries; moduleEntry != nullptr; moduleEntry = moduleEntry->next)
  {
    io::print("addr: ", (uintptr_t)moduleEntry->addr, ", len: ", moduleEntry->len, "\n");

    core::Process process;

    process.setAsActive();
    process.addSection(0x00000000, core::memory::Access::ALL, core::memory::Permission::READ_ONLY, 
        reinterpret_cast<const uint8_t*>(moduleEntry->addr), moduleEntry->len);

    asm volatile ( R"(
      .intel_syntax noprefix
        push 0x23
        push 0x00000000
        pushf
        push 0x1B
        push 0x00000000
        iret
      .att_syntax prefix
      )"
      :
      : 
      :
    );
  }


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

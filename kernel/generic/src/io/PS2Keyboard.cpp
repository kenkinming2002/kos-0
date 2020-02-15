#include <generic/io/PS2Keyboard.hpp>

#include <i686/core/Interrupt.hpp>

#include <intel/core/pic/8259.hpp>
#include <intel/asm/io.hpp>

#include <generic/io/Print.hpp>

#include <generic/io/PS2Controller.hpp>

namespace io
{
  namespace
  {
    [[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void keyboard_interrupt_handler([[maybe_unused]]core::interrupt::frame* frame)
    {
      uint8_t scanCode = assembly::inb(0x60);
      ps2Keyboard.push(scanCode);

      core::pic::controller8259::acknowledge(0x1);
    }
  }

  PS2Keyboard::PS2Keyboard()
  {
    core::interrupt::install_handler(0x21, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&keyboard_interrupt_handler));
    // TODO: Verify that device 1 is keyboard
    core::pic::controller8259::clearMask(1); // Enable keyboard
  }

  void PS2Keyboard::push(uint8_t scanCode)
  {
    m_buffer.push(scanCode);
  }

  std::optional<uint8_t> PS2Keyboard::poll()
  {
    std::optional<uint8_t> res;

    // TODO: handle concurency
    if(!m_buffer.empty())
      res = m_buffer.pop();

    return res;
  }

  PS2Keyboard ps2Keyboard;
}

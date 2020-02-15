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
      io::print("KEYBOARD INTERRUPT");

      uint8_t scanCode = assembly::inb(0x60);
      io::print("Scan code:", (uint16_t)scanCode);

      io::print("KEYBOARD INTERRUPT acknowledged");
      core::pic::controller8259::acknowledge(0x1);
    }

    [[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void mouse_interrupt_handler([[maybe_unused]]core::interrupt::frame* frame)
    {
      io::print("MOUSE INTERRUPT");

      uint8_t scanCode = assembly::inb(0x60);
      io::print("Scan code:", (uint16_t)scanCode);

      io::print("MOUSE INTERRUPT acknowledged");
      core::pic::controller8259::acknowledge(0xC);
    }
  }

  PS2Keyboard::PS2Keyboard()
  {
    core::interrupt::install_handler(0x21, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&keyboard_interrupt_handler));
    core::interrupt::install_handler(0x2D, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&mouse_interrupt_handler));
    // TODO: Verify that device 1 is keyboard
    core::pic::controller8259::clearMask(1); // Enable keyboard
  }

  PS2Keyboard ps2Keyboard;
}

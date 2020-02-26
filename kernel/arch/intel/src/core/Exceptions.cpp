

#include <i686/core/Interrupt.hpp>
#include <generic/io/Print.hpp>

namespace core::exceptions
{
  namespace
  {
    [[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void double_fault_handler([[maybe_unused]]core::interrupt::frame* frame, 
        core::interrupt::uword_t error_code)
    {
      CORE_INTERRUPT_ENTRY;
      io::print("Double Fault - error code: ", (uint32_t)error_code); 
      for(;;)
        asm("hlt");
    }

    [[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void stack_segment_fault_handler([[maybe_unused]]core::interrupt::frame* frame,
        core::interrupt::uword_t error_code)
    {
      CORE_INTERRUPT_ENTRY;
      io::print("Stack Segment Fault", (uint32_t)error_code); 
      for(;;)
        asm("hlt");
    }

    [[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void general_pretection_fault_handler(
        [[maybe_unused]]core::interrupt::frame* frame, core::interrupt::uword_t error_code)
    {
      CORE_INTERRUPT_ENTRY;
      io::print("General Protection Fault - error code: ", (uint32_t)error_code); 
      for(;;)
        asm("hlt");
    }
  }

  void init()
  {
    core::interrupt::install_handler(0x08, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&double_fault_handler));
    core::interrupt::install_handler(0x0C, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&stack_segment_fault_handler));
    core::interrupt::install_handler(0x0D, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&general_pretection_fault_handler));
  }
}

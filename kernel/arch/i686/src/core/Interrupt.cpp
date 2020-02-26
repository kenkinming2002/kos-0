#include <i686/core/Interrupt.hpp>

#include <intel/asm/idt.hpp>

/*
 * On x86, when an interrupt occurs, if the handler is going to be executed at a
 * numerically lower level, a task switch occurs.
 *
 * a) The stack segment selector and stack pointer of the interrupted procedure
 *    is pushed on the stack.
 * b) The stack segment selector and stack pointer of the handler procedure of
 *    appropriate privillege level is obtained from the TSS.
 *
 *  In all cases,
 *
 * c) The EFLAGS, code segment selector and instruction pointer of the
 *    interrupted procedure is pushed on the stack.
 * d) If the interrupt is caused by an exception with an ERROR code, it is
 *    pushed on the stack.
 *
 * Note that in all cases, only stack segment selector, stack pointer, code
 * segment selector and instruction pointer is saved and restored, while other
 * segment selectors is untouched.
 *
 * Thus, it is necessary to save and restore other segment selectors ourself.
 *
 * Refer to section 6.12.1 of Intel Programming Guide Volume 3A
 */

namespace core::interrupt
{
  IDTEntry::IDTEntry(InterruptType interruptType, PrivillegeLevel privillegeLevel, uint16_t selector, Handler irqHandler, bool present)
  {
    m_typeAttr = static_cast<uint8_t>(interruptType)        | 
                 static_cast<uint8_t>(privillegeLevel) << 5 | 
                 static_cast<uint8_t>(present)         << 7;
    m_selector = selector;

    m_offsetLow  = reinterpret_cast<uintptr_t>(irqHandler) & 0xFFFF;
    m_offsetHigh = reinterpret_cast<uintptr_t>(irqHandler) >> 16;

    m_zero = 0;
  }

  bool IDTEntry::enabled() const
  {
    return m_typeAttr & (1u << 7);
  }

  void IDTEntry::enabled(bool state)
  {
    bool success;
    do {
      auto mask = ~(static_cast<uint8_t>(state) << 7);
      auto oldValue = m_typeAttr;
      auto newValue = (oldValue & mask) | (static_cast<uint8_t>(state) << 7);

      success = __sync_bool_compare_and_swap(&m_typeAttr, oldValue, newValue);
    } while(!success);
  }

  IDT::IDT(const IDTEntry idtEntries[256])
  {
    m_size = sizeof(IDTEntry) * 256 - 1;
    m_offset = reinterpret_cast<uintptr_t>(idtEntries);
  }

  int IDT::load() const
  {
    assembly::lidt(reinterpret_cast<const void*>(this));
    return 0;
  }

  namespace
  {
    constexpr static size_t IDT_SIZE = 256;
    __attribute__((init_priority(101))) IDTEntry idtEntries[IDT_SIZE];
  }

  void init()
  {
    IDT(idtEntries).load();
  }

  int install_handler(int irqNumber, PrivillegeLevel privillegeLevel, uintptr_t handler)
  {
    if(idtEntries[irqNumber].enabled())
      return -1;

    idtEntries[irqNumber] = IDTEntry(InterruptType::INTERRUPT_GATE_32, privillegeLevel, 0x08, handler, true);
    return 0;
  }

  void reinstall_handler(int irqNumber, PrivillegeLevel privillegeLevel, uintptr_t handler)
  {
    idtEntries[irqNumber] = IDTEntry(InterruptType::INTERRUPT_GATE_32, privillegeLevel, 0x08, handler, true);
  }

  void uninstall_handler(int irqNumber)
  {
    idtEntries[irqNumber].enabled(false);
  }
}

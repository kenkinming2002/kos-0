#include <core/i686/interrupts/IDT.hpp>


#include <stdint.h>
#include <stddef.h>

namespace core::interrupts
{
  IDTEntry::IDTEntry(InterruptType interruptType, PrivilegeLevel privillegeLevel, uint16_t selector, uintptr_t address, bool present)
  {
    m_typeAttr = static_cast<uint8_t>(interruptType)        | 
                 static_cast<uint8_t>(privillegeLevel) << 5 | 
                 static_cast<uint8_t>(present)         << 7;
    m_selector = selector;

    m_offsetLow  = address & 0xFFFF;
    m_offsetHigh = address >> 16;

    m_zero = 0;
  }

  bool IDTEntry::enabled() const
  {
    return m_typeAttr & (1u << 7);
  }

  void IDTEntry::enabled(bool state)
  {
    // FIXME: I don't know why I am doing this __sync_bool_compare_and_swap
    //        dance.
    bool success;
    do {
      auto mask = ~(static_cast<uint8_t>(state) << 7);
      auto oldValue = m_typeAttr;
      auto newValue = (oldValue & mask) | (static_cast<uint8_t>(state) << 7);

      success = __sync_bool_compare_and_swap(&m_typeAttr, oldValue, newValue);
    } while(!success);
  }

  IDT::IDT(const IDTEntry(&idtEntries)[256])
  {
    m_size = sizeof idtEntries - 1;
    m_offset = reinterpret_cast<uintptr_t>(idtEntries);
  }
}

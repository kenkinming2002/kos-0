#pragma once

#include <core/i686/PrivilegeLevel.hpp>

namespace core::interrupts
{
  enum class InterruptType
  {
    TASK_GATE_32      = 0x5,
    INTERRUPT_GATE_16 = 0x6,
    TRAP_GATE_16      = 0x7, 
    INTERRUPT_GATE_32 = 0xE, 
    TRAP_GATE_32      = 0xF
  };

  class IDTEntry
  {
  public:
    IDTEntry() = default;
    IDTEntry(InterruptType interruptType, PrivilegeLevel privillegeLevel, uint16_t selector, uintptr_t address, bool present = true);

  public:
    bool enabled() const;
    void enabled(bool state);

  private:
    uint16_t m_offsetLow  = 0; 
    uint16_t m_selector   = 0; 
    uint8_t  m_zero       = 0;      
    uint8_t  m_typeAttr   = 0;
    uint16_t m_offsetHigh = 0; 
  }__attribute((packed));

  struct IDT
  {
  public:
    IDT(const IDTEntry(&idtEntries)[256]);

  private:
    uint16_t m_size;
    uint32_t m_offset;
  }__attribute__((packed));
}

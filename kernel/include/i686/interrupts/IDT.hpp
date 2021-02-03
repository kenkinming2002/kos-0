#pragma once

#include <i686/PrivilegeLevel.hpp>

namespace core::interrupts
{
  enum Mask
  {
    TYPE_MASK            = 0xF,
    PRIVILEGE_LEVEL_MASK = 0x60,
    PRESENT_MASK         = 0x80
  };

  enum Shift
  {
    TYPE_SHIFT = 0,
    PRIVILEGE_LEVEL_SHIFT = 5,
    PRESENT_SHIFT = 7
  };

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
    constexpr IDTEntry() = default;
    constexpr IDTEntry(InterruptType interruptType, PrivilegeLevel privilegeLevel, uint16_t selector, uintptr_t address, bool present = true)
    {
      m_typeAttr = static_cast<uint8_t>(interruptType)  << TYPE_SHIFT |
                   static_cast<uint8_t>(privilegeLevel) << PRIVILEGE_LEVEL_SHIFT |
                   static_cast<uint8_t>(present)        << PRESENT_SHIFT;
      m_selector = selector;

      m_offsetLow  = address & 0xFFFF;
      m_offsetHigh = address >> 16;

      m_zero = 0;
    }

  public:
    constexpr bool present() const { return (m_typeAttr & PRESENT_MASK) >> PRESENT_SHIFT; }
    constexpr void present(bool state) { m_typeAttr = (static_cast<uint8_t>(m_typeAttr) & ~PRESENT_MASK) | (static_cast<uint8_t>(state) << PRESENT_SHIFT); }

    constexpr InterruptType type() const { return static_cast<InterruptType>((m_typeAttr & TYPE_MASK) >> TYPE_SHIFT); }
    constexpr void type(InterruptType type) { m_typeAttr = (static_cast<uint8_t>(m_typeAttr) & ~TYPE_MASK) | (static_cast<uint8_t>(type) << TYPE_SHIFT); }

    constexpr PrivilegeLevel privilegeLevel() const { return static_cast<PrivilegeLevel>((m_typeAttr & PRIVILEGE_LEVEL_MASK) >> PRIVILEGE_LEVEL_SHIFT); }
    constexpr void privilegeLevel(PrivilegeLevel privilegeLevel) { m_typeAttr = (static_cast<uint8_t>(m_typeAttr) & ~PRIVILEGE_LEVEL_MASK) | (static_cast<uint8_t>(privilegeLevel) << PRIVILEGE_LEVEL_SHIFT); }

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
    IDT(const IDTEntry(&idtEntries)[256])
    {
      m_size = sizeof idtEntries - 1;
      m_offset = reinterpret_cast<uintptr_t>(idtEntries);
    }

  private:
    uint16_t m_size;
    uint32_t m_offset;
  }__attribute__((packed));
}

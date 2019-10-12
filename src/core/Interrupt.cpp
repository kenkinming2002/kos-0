#include <core/Interrupt.hpp>

#include <asm/idt.h>

IDTEntry::IDTEntry(InterruptType interruptType, PrivillegeLevel privillegeLevel, uint16_t selector, irq_handler_t irq_handler)
{
  m_typeAttr = static_cast<uint8_t>(interruptType) | static_cast<uint8_t>(privillegeLevel) << 5 | 1u << 7;
  m_selector = selector;

  m_offsetLow  = reinterpret_cast<uint32_t>(irq_handler) & 0xFFFF;
  m_offsetHigh = reinterpret_cast<uint32_t>(irq_handler) >> 16;

  m_zero = 0;
}

IDT::IDT(const IDTEntry idtEntries[256])
{
  m_size = sizeof(IDTEntry) * 256 - 1;
  m_offset = reinterpret_cast<uint32_t>(idtEntries);
}

int IDT::load() const
{
  lidt(reinterpret_cast<const void*>(this));
  return 0;
}

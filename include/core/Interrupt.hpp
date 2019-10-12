#pragma once

#include <stdint.h>
#include <core/Segmentation.hpp>

enum InterruptType
{
  TASK_GATE_32      = 0x05,
  INTERRUPT_GATE_16 = 0x6,
  TRAP_GATE_16      = 0x7,

  INTERRUPT_GATE_32 = 0xE,
  TRAP_GATE_32      = 0xF
};

typedef void* irq_handler_t;

class IDTEntry
{
public:
  IDTEntry() = default;
  IDTEntry(InterruptType interruptType, PrivillegeLevel privillegeLevel, uint16_t selector, irq_handler_t irq_handler);

private:
  uint16_t m_offsetLow; 
  uint16_t m_selector; 
  uint8_t  m_zero;      
  uint8_t  m_typeAttr;
  uint16_t m_offsetHigh; 
}__attribute((packed));

struct IDT
{
public:
  IDT(const IDTEntry idtEntries[256]);

public:
  int load() const;

private:
  uint16_t m_size;
  uint32_t m_offset;
}__attribute__((packed));


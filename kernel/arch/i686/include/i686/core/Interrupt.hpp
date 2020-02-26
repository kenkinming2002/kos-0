#pragma once

#include <i686/core/Segmentation.hpp>

#include <stdint.h>
#include <utility>

namespace core::interrupt
{
  enum class InterruptType
  {
    TASK_GATE_32      = 0x05,
    INTERRUPT_GATE_16 = 0x6,
    TRAP_GATE_16      = 0x7, INTERRUPT_GATE_32 = 0xE, TRAP_GATE_32      = 0xF
  };

  using Handler = uintptr_t;

  class IDTEntry
  {
  public:
    IDTEntry() = default;
    IDTEntry(InterruptType interruptType, PrivillegeLevel privillegeLevel, uint16_t selector, Handler irqHandler, bool present = false);

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
    IDT(const IDTEntry idtEntries[256]);

  public:
    int load() const;

  private:
    uint16_t m_size;
    uint32_t m_offset;
  }__attribute__((packed));

// Save current data segment register and load kernel data segment register 
#define CORE_INTERRUPT_ENTRY      \
  uint16_t ds;                    \
  asm volatile (                  \
    ".intel_syntax noprefix \t\n" \
    "  mov %[ds], ds        \t\n" \
    "  mov ax, 0x10         \t\n" \
    "  mov ds, ax           \t\n" \
    "  mov es, ax           \t\n" \
    "  mov fs, ax           \t\n" \
    "  mov gs, ax           \t\n" \
    ".att_syntax prefix     \t\n" \
    : [ds]"=rm"(ds)               \
    :                             \
    : "ax"                        \
  )                               \

// Restore data segment register
#define CORE_INTERRUPT_EXIT       \
  asm volatile (                  \
    ".intel_syntax noprefix \t\n" \
    "  mov ax, %[ds]        \t\n" \
    "  mov ds, ax           \t\n" \
    "  mov es, ax           \t\n" \
    "  mov fs, ax           \t\n" \
    "  mov gs, ax           \t\n" \
    ".att_syntax prefix     \t\n" \
    :                             \
    : [ds]"rm"(ds)                \
    : "ax"                        \
  )                               \

#ifdef __x86_64__
  typedef unsigned long long int uword_t;
#else
  typedef unsigned int uword_t;
#endif

  struct frame
  {
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t sp;
    uint32_t ss;
  };

  void init();

  int install_handler(int irqNumber, PrivillegeLevel privillegeLevel, uintptr_t irqHandler);
  void reinstall_handler(int irqNumber, PrivillegeLevel privillegeLevel, uintptr_t irqHandler);
  void uninstall_handler(int irqNumber);
}

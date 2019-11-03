#pragma once

#include <i686/core/Segmentation.hpp>

#include <stdint.h>
#include <utility>

namespace core::i686
{
  enum class InterruptType
  {
    TASK_GATE_32      = 0x05,
    INTERRUPT_GATE_16 = 0x6,
    TRAP_GATE_16      = 0x7,

    INTERRUPT_GATE_32 = 0xE,
    TRAP_GATE_32      = 0xF
  };

  using Handler = void(*)();

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
}

namespace core
{
  using Handler = void(*)();

  class Interrupt
  {
  public:
    Interrupt();

  public:
    int installHandler(int irqNumber, PrivillegeLevel privillegeLevel, Handler irqHandler);
    void reinstallHandler(int irqNumber, PrivillegeLevel privillegeLevel, Handler irqHandler);
    void uninstallHandler(int irqNumber);

  private:
    void enabled(int irqNumber, bool state);
    bool enabled(int irqNumber) const;

  private:
    void initEntries();

    template<size_t... Is>
    void initEntries(std::index_sequence<Is...>);

    template<size_t I>
    void initEntry();

    template<size_t I>
    __attribute__((naked)) static void rawHandler();

  public:
    constexpr static size_t IDT_SIZE = 256;


  //SUGGESTION: If needed m_irqHandlers can be converted to non-static member,
  //by using a global pointer into the currently active Interrupt class.
  //However, that will incur runtime cost, which may not be desired.
  private:
    i686::IDTEntry m_idtEntries[IDT_SIZE];
    static Handler m_irqHandlers[IDT_SIZE];
  };
}

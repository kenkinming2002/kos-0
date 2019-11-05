#include <i686/core/Interrupt.hpp>

#include <intel/asm/idt.hpp>

namespace core::i686
{
  IDTEntry::IDTEntry(InterruptType interruptType, PrivillegeLevel privillegeLevel, uint16_t selector, Handler irqHandler, bool present)
  {
    m_typeAttr = static_cast<uint8_t>(interruptType)        | 
                 static_cast<uint8_t>(privillegeLevel) << 5 | 
                 static_cast<uint8_t>(present)         << 7;
    m_selector = selector;

    m_offsetLow  = reinterpret_cast<uint32_t>(irqHandler) & 0xFFFF;
    m_offsetHigh = reinterpret_cast<uint32_t>(irqHandler) >> 16;

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
    m_offset = reinterpret_cast<uint32_t>(idtEntries);
  }

  int IDT::load() const
  {
    assembly::lidt(reinterpret_cast<const void*>(this));
    return 0;
  }
}

namespace core
{
  utils::Callback Interrupt::m_irqHandlers[IDT_SIZE];

  Interrupt::Interrupt()
  {
    initEntries();

    i686::IDT idt(m_idtEntries);
    idt.load();
  }

  int Interrupt::installHandler(int irqNumber, PrivillegeLevel privillegeLevel, utils::Callback irqHandler)
  {
    if(this->enabled(irqNumber))
      return -1;
    m_irqHandlers[irqNumber] = irqHandler; // Set up irqHandler
    this->enabled(irqNumber, true);

    return 0;
  }

  void Interrupt::reinstallHandler(int irqNumber, PrivillegeLevel privillegeLevel, utils::Callback irqHandler)
  {
    m_irqHandlers[irqNumber] = irqHandler; // Set up irqHandler
    this->enabled(irqNumber, true);
  }

  void Interrupt::uninstallHandler(int irqNumber)
  {
    m_irqHandlers[irqNumber] = nullptr;
    this->enabled(irqNumber, false);
  }

  void Interrupt::enabled(int irqNumber, bool state)
  {
    m_idtEntries[irqNumber].enabled(state);
  }

  bool Interrupt::enabled(int irqNumber) const
  {
    return m_idtEntries[irqNumber].enabled();
  }

  void Interrupt::initEntries()
  {
    initEntries(std::make_index_sequence<IDT_SIZE>{});
  }

  template<size_t... Is>
  void Interrupt::initEntries(std::index_sequence<Is...>)
  {
    (initEntry<Is>(), ...);
  }

  template<size_t I>
  void Interrupt::initEntry()
  {
    m_idtEntries[I] = i686::IDTEntry(i686::InterruptType::INTERRUPT_GATE_32, PrivillegeLevel::RING0, 0x08, &rawHandler<I>);
  }

  template<size_t I>
  void Interrupt::rawHandler()
  {
      m_irqHandlers[I]();
      asm("iret");
  }
}

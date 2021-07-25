#include <x86/interrupts/PIC.hpp>

#include <x86/interrupts/8259.hpp>
#include <x86/interrupts/APIC.hpp>

namespace core::interrupts
{
  namespace
  {
    constinit PIC* pic = nullptr;
  }

  void initializePIC()
  {
    core::interrupts::initializePIC8259();
    core::interrupts::initializeAPIC();
  }

  void registerPIC(PIC& _pic) { pic = &_pic; }

  void mask(irq_t irq)
  {
    pic->mask(irq);
  }

  void unmask(irq_t irq)
  {
    pic->unmask(irq);
  }

  void acknowledge(irq_t irq)
  {
    pic->acknowledge(irq);
  }

  irq_t translateISA(unsigned isa)
  {
    return pic->translateISA(isa);
  }

  irq_t translateGSI(unsigned gsi)
  {
    return pic->translateGSI(gsi);
  }

  void addTimerCallback(PICTimer::callback_t callback)
  {
    pic->timer().registerCallback(callback);
  }
}

#pragma once

#include <i686/interrupts/Interrupts.hpp>

namespace core::interrupts
{
  /* There is two translation with regard to ISA irq.
   *
   * ISA irq number -> GSI irq number -> CPU irq number
   *
   * When using single/dual-8259 PIC controller, the first translation is
   * simpler an identity map, whereas the second translation is controlled by
   * offset of the PIC controller.
   *
   * When using an IOAPIC, the first translation could be located in MADT and is
   * hardware specififc whereas the second translation is is controlled by the
   * redirection entries on the IOAPIC.
   */

  class PIC
  {
  /* hardware interrupts */
  public:
    /* Offset required to avoid colliding with interrupt vectors used for cpu
     * exceptions */
    constexpr static irq_t OFFSET = 0x20;

  public:
    virtual void mask(irq_t irq) = 0;
    virtual void unmask(irq_t irq) = 0;
    virtual void acknowledge(irq_t irq) = 0;

  public:
    virtual irq_t translateISA(unsigned isa) = 0;
    virtual irq_t translateGSI(unsigned gsi) = 0;

  public:
    virtual ~PIC() = default;
  };


  /*
   * The interrupts first 0x20 interrupts are reserved for cpu exceptions
   */
  void initializePIC();
  void registerPIC(PIC& pic);

  void mask(irq_t irq);
  void unmask(irq_t irq);
  void acknowledge(irq_t irq);

  static constexpr size_t ISA_IRQ_COUNT = 16;
  irq_t translateISA(unsigned isa);

  /* We have 256 interrupt vectors but, the first 32 irq is reserved by cpu
   * exceptions and the last one is reserved for spurious interrupt for Local
   * APIC */
  static constexpr size_t GSI_IRQ_COUNT = 223;
  irq_t translateGSI(unsigned gsi);
}

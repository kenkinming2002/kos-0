#include <x86/interrupts/APIC.hpp>

#include <x86/interrupts/PIC.hpp>
#include <x86/interrupts/IOAPIC.hpp>
#include <x86/LocalAPIC.hpp>
#include <x86/acpi/ACPI.hpp>

#include <x86/assembly/msr.hpp>

#include <generic/Init.hpp>

#include <librt/Panic.hpp>
#include <librt/Log.hpp>
#include <librt/Global.hpp>

#include <librt/containers/StaticVector.hpp>

namespace core::interrupts
{
  class APIC : public PIC
  {
  public:
    static constexpr uint32_t IA32_APIC_BASE_MSR        = 0x1B;
    static constexpr uint64_t IA32_APIC_BASE_MSR_ENABLE = 0x800;
    static constexpr irq_t SPURIOUS_INTERRUPT_VECTOR = 0xFF;

  public:
    APIC()
    {
      /* Assume identity mapping from isa irq to gsi until otherwise specified in
       * MADT */
      for(uint32_t i=0; i<ISA_IRQ_COUNT; ++i)
        m_isaTranslations[i] = i;

      /* Parse the MADT */
      auto* madt = static_cast<const MADT*>(acpi::findSDT("APIC"));
      if(!madt)
        rt::panic("Failed to find MADT\n");

      for(auto* madtEntry = madt->entries; madtEntry<MADTEntryEnd(madt); madtEntry = MADTEntryNext(madtEntry))
        switch(madtEntry->type)
        {
          case MADTEntry::Type::IO_APIC:
          {
            auto* entry = static_cast<const IOAPICMADTEntry*>(madtEntry);
            m_ioapics.pushBack(IOAPIC(entry->IOAPICAddress, entry->gsiBase));
            break;
          }
          case MADTEntry::Type::IO_APIC_INTERRUPT_SOURCE_OVERRIDE:
          {
            auto* entry = static_cast<const IOAPICInterruptSourceOverrideMADTEntry*>(madtEntry);
            if(entry->busSource != 0)
              rt::panic("Unrecognized bus source for IOAPIC Interrupt Source Override\n");

            m_isaTranslations[entry->irqSource] = entry->gsi;
            m_gsiConfigs[entry->gsi] = GSIConfig(entry->flags);
            break;
          }
          default:
            break;
        }

      foreachCPUInitCall([]() {
        assembly::wrmsr(IA32_APIC_BASE_MSR, assembly::rdmsr(IA32_APIC_BASE_MSR) | IA32_APIC_BASE_MSR_ENABLE);
        LocalAPIC::write(LocalAPIC::SPURIOUS_INTERRUPT_VECTOR_REGISTER, 0x1 << 8 | SPURIOUS_INTERRUPT_VECTOR);
      });
    }

  private:
    unsigned toGSI(irq_t irq) { return irq - OFFSET; }

  public:
    IOAPIC& findIOAPIC(unsigned gsi)
    {
      for(auto& ioapic : m_ioapics)
        if(ioapic.handle(gsi))
          return ioapic;

      ASSERT_UNREACHABLE;
    }

  public:
    void mask(irq_t irq) override
    {
      auto gsi = toGSI(irq);
      auto& ioapic = findIOAPIC(gsi);
      ioapic.writeRedirectionEntry(gsi, RedirectionEntry());
    }

    void unmask(irq_t irq) override
    {
      auto gsi = toGSI(irq);
      auto& ioapic = findIOAPIC(gsi);

      // TODO: Dynamically balance irq to different processor core
      ioapic.writeRedirectionEntry(gsi, RedirectionEntry(irq,
        DeliveryMode::FIXED, DestinationMode::PHYSICAL,
        m_gsiConfigs[gsi].polarity, m_gsiConfigs[gsi].triggerMode, 0));
    }

    void acknowledge(irq_t irq) override
    {
      // Consider checking if it is indeed the current interrupt
      LocalAPIC::write(LocalAPIC::EOI_REGISTER, 0);
    }

  public:
    irq_t translateISA(unsigned isa) override { return translateGSI(m_isaTranslations[isa]); }
    irq_t translateGSI(unsigned gsi) override { return OFFSET + gsi; }

  private:
    unsigned m_isaTranslations[ISA_IRQ_COUNT];
    struct GSIConfig
    {
      /* Assume active high and edge trggered, this may change in the future */
      Polarity polarity       : 1 = Polarity::ACTIVE_HIGH;
      TriggerMode triggerMode : 1 = TriggerMode::EDGE;

      constexpr GSIConfig() = default;
      constexpr GSIConfig(uint16_t flags)
      {
        switch((flags & 0b0011) >> 0)
        {
          case 0b01: polarity = Polarity::ACTIVE_HIGH; break;
          case 0b11: polarity = Polarity::ACTIVE_LOW;  break;
          default:   polarity = Polarity::ACTIVE_HIGH; break;
        }

        switch((flags & 0b1100) >> 2)
        {
          case 0b01: triggerMode = TriggerMode::EDGE;  break;
          case 0b11: triggerMode = TriggerMode::LEVEL; break;
          default:   triggerMode = TriggerMode::EDGE;  break;
        }
      }
    };

  private:
    GSIConfig m_gsiConfigs[GSI_IRQ_COUNT];
    rt::containers::StaticVector<IOAPIC, 4> m_ioapics;
  };

  namespace { constinit rt::Global<APIC> apic; }

  void initializeAPIC()
  {
    apic.construct();
    registerPIC(apic());
  }
}

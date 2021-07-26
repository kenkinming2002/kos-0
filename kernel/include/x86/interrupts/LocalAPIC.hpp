#pragma once

#include <x86/assembly/msr.hpp>

#include <generic/memory/Virtual.hpp>

#include <librt/Log.hpp>

#include <stddef.h>
#include <stdint.h>

namespace core::interrupts
{
  class LocalAPIC
  {
  public:
    static constexpr irq_t TIMER_VECTOR              = 0xFE;
    static constexpr irq_t SPURIOUS_INTERRUPT_VECTOR = 0xFF;

  public:
    static constexpr uint32_t IA32_APIC_BASE_MSR        = 0x1B;
    static constexpr uint64_t IA32_APIC_BASE_MSR_ENABLE = 0x800;

  public:
    static constexpr size_t SPURIOUS_INTERRUPT_VECTOR_REGISTER = 0x0F0;
    static constexpr size_t EOI_REGISTER                       = 0x0B0;

    static constexpr size_t LVT_TIMER_REGISTER                  = 0x320;
    static constexpr size_t TIMER_INITIAL_COUNT_REGISTER        = 0x380;
    static constexpr size_t TIMER_CURRENT_COUNT_REGISTER        = 0x390;
    static constexpr size_t TIMER_DIVIDE_CONFIGURATION_REGISTER = 0x3E0;

  public:
    constexpr LocalAPIC(memory::physaddr_t physaddr) : m_physaddr(physaddr) {}

  public:
    void enable()
    {
      rt::logf("Enabling Local APIC\n");
      assembly::wrmsr(IA32_APIC_BASE_MSR, assembly::rdmsr(IA32_APIC_BASE_MSR) | IA32_APIC_BASE_MSR_ENABLE);
      write(SPURIOUS_INTERRUPT_VECTOR_REGISTER, 0x1 << 8 | SPURIOUS_INTERRUPT_VECTOR);
    }

    void enableTimer()
    {
      rt::logf("Enabling Local APIC Timer\n");
      write(LVT_TIMER_REGISTER, 0x0 << 17 /* One-shot mode */ | TIMER_VECTOR);
      write(TIMER_DIVIDE_CONFIGURATION_REGISTER, 0b1000); // divied by 32
    }

    void resetTimer()
    {
      write(TIMER_INITIAL_COUNT_REGISTER, 0x1000); // Set initial count: TODO: Allow adjusting
    }

  public:
    void acknowledge(irq_t irq) { write(EOI_REGISTER, 0); }

  public:
    void write(size_t offset, uint32_t data)
    {
      memory::KMapGuard guard(m_physaddr);
      *reinterpret_cast<uint32_t*>(guard.addr+offset) = data;
    }

    uint32_t read(size_t offset)
    {
      memory::KMapGuard guard(m_physaddr);
      return *reinterpret_cast<uint32_t*>(guard.addr+offset);
    }

  private:
    memory::physaddr_t m_physaddr;
  };
}

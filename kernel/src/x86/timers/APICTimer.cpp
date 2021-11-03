#include <generic/timers/Timer.hpp>

#include <x86/assembly/io.hpp>

#include <x86/LocalAPIC.hpp>
#include <i686/interrupts/Interrupts.hpp>
#include <x86/interrupts/PIC.hpp>
#include <generic/Init.hpp>

#include <librt/Log.hpp>
#include <librt/Global.hpp>

namespace core::timers
{
  class APICTimer : public Timer
  {
  public:
    static constexpr irq_t TIMER_VECTOR = 0xFE;

  private:
    void calibrate()
    {
      // We use PIT Channel 2 to calibrate
      uint32_t begin = -1, end;

      // Configure PIT
      assembly::outb(0x61, (assembly::inb(0x61) & 0xFC) | 0x1);
      assembly::outb(0x43, 0b10110010); // Channel 2, Access Mode: lobyte/hibyte, Hardware Retriggerable One-shot, Binary
      assembly::outb(0x42, 0x9b);
      assembly::ioWait();
      assembly::outb(0x42, 0x2e);

      // Reset PIT
      auto b = assembly::inb(0x61);
      assembly::outb(0x61, (b & 0xFE) | 0x0);
      assembly::outb(0x61, (b & 0xFE) | 0x1);

      // Reset APIC Timer
      LocalAPIC::write(LocalAPIC::TIMER_INITIAL_COUNT_REGISTER, begin);

      // Wait
      while(!(assembly::inb(0x61) & 0x20));

      // Read the result
      end = LocalAPIC::read(LocalAPIC::TIMER_CURRENT_COUNT_REGISTER);

      // Stop the APIC Timer
      LocalAPIC::write(LocalAPIC::TIMER_INITIAL_COUNT_REGISTER, 0);

      auto ticks = begin - end;
      rt::logf("APIC Timer runs at %lu ticks per seconds\n", ticks * 100);
    }

  public:
    APICTimer()
    {
      foreachCPUInitCall([](){
        LocalAPIC::write(LocalAPIC::LVT_TIMER_REGISTER, 0x0 << 17 /* One-shot mode */ | TIMER_VECTOR);
        LocalAPIC::write(LocalAPIC::TIMER_DIVIDE_CONFIGURATION_REGISTER, 0b1011); // divied by 1
      });
      calibrate();
      interrupts::installHandler(TIMER_VECTOR, &timerHandler, PrivilegeLevel::RING0, true);
    }

  private:
    static inline constinit callback_t m_callback;
    static inline void* m_data;

  public:
    static void timerHandler(irq_t irq, uword_t, uintptr_t)
    {
      interrupts::acknowledge(irq);
      if(m_callback)
        m_callback(m_data);
    }

  public:
    rt::StringRef name() override { return "APICTimer"; }
    void addCallback(callback_t callback, void* data) override
    {
      m_callback = callback;
      m_data = data;
    }

    void reset() override
    {
      LocalAPIC::write(LocalAPIC::TIMER_INITIAL_COUNT_REGISTER, 0x1000); // Set initial count: TODO: Allow adjusting
    }
  };

  namespace
  {
    rt::Global<APICTimer> apicTimer;
  }

  void initializeAPICTimer()
  {
    apicTimer.construct();
    registerTimer(apicTimer());
  }
}

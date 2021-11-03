#include <generic/timers/Timer.hpp>

#include <i686/interrupts/Interrupts.hpp>
#include <x86/interrupts/PIC.hpp>
#include <generic/Init.hpp>

#include <librt/Global.hpp>
#include <librt/Log.hpp>

namespace core::timers
{
  class PIT : public Timer
  {
  public:
    PIT()
    {
      auto irq = interrupts::translateISA(0);
      interrupts::unmask(irq);
      interrupts::installHandler(irq, &timerHandler, PrivilegeLevel::RING0, true);
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
    rt::StringRef name() override { return "PIT"; }
    void addCallback(callback_t callback, void* data) override
    {
      m_callback = callback;
      m_data = data;
    }

    void reset() override
    {
      // We Actually should setup PIT in one-shot mode, but who cares?
    }
  };

  namespace
  {
    rt::Global<PIT> pitTimer;
  }

  void initializePIT()
  {
    pitTimer.construct();
    registerTimer(pitTimer());
  }
}


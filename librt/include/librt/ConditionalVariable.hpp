#pragma once

#include <atomic>

namespace rt
{
  inline void pause() { asm volatile("pause"); }
  class ConditionalVariable
  {
  public:
    void notify(size_t count = 1)
    {
      for(size_t i=0; i<count; ++i)
      {
        m_flag.store(true, std::memory_order_release);
        while(m_flag.load(std::memory_order_acquire)) pause();
      }
    }

  public:
    void wait()
    {
      while(!m_flag.exchange(false, std::memory_order_acq_rel)) pause();
    }

  private:
    std::atomic<bool> m_flag = false;
  };
}

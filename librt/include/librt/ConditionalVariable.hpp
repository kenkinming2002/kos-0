#pragma once

#include <atomic>

namespace rt
{
  inline void pause() { asm volatile("pause"); }

  class ConditionalVariable
  {
  public:
    void notify(unsigned total)
    {
      // (1)
      count.store(0, std::memory_order_release);
      flag.store(true, std::memory_order_release);

      // (3)
      while(count.load(std::memory_order_acquire) != total) pause();
      flag.store(false, std::memory_order_release);
    }

    void wait()
    {
      // (2)
      while(!flag.load(std::memory_order_acquire)) pause();
      count.fetch_add(1, std::memory_order_release);

      // (4)
      while(flag.load(std::memory_order_acquire)) pause(); // (7)
    }

  private:
    std::atomic<bool> flag = false;
    std::atomic<unsigned> count = 0;
  };
}

#pragma once

#include <atomic>

namespace rt
{
  inline void pause() { asm volatile("pause"); }
  class SpinLock
  {
  public:
    void lock() { while(m_flag.test_and_set()) pause(); }
    void unlock() { m_flag.clear(); }

  private:
    std::atomic_flag m_flag;
  };

  template<typename Lock>
  class LockGuard
  {
  public:
    LockGuard(Lock& lock) : m_lock(lock) { m_lock.lock(); }
    ~LockGuard() { m_lock.unlock(); }

  private:
    Lock& m_lock;
  };
}

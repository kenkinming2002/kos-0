#pragma once

#include <librt/UniquePtr.hpp>
#include <generic/SpinLock.hpp>

namespace core
{
  void initializePerCPU();

  unsigned cpuidCurrent();
  unsigned getCpusCount();

  template<typename T>
  class PerCPU
  {
  public:
    constexpr PerCPU() = default;

  private:
    void ensure() const
    {
      core::LockGuard guard(m_lock);
      if(!m_values)
        m_values = rt::makeUnique<T[]>(getCpusCount());
    }

  public:
    const T& get(unsigned cpuid) const { ensure(); return m_values[cpuid]; }
    T& get(unsigned cpuid)             { ensure(); return m_values[cpuid]; }

  public:
    const T& current() const { return get(cpuidCurrent()); }
    T& current()             { return get(cpuidCurrent()); }

  private:
    mutable core::SpinLock m_lock;
    mutable rt::UniquePtr<T[]> m_values = nullptr;
  };
}

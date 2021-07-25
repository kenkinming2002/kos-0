#pragma once

#include <librt/UniquePtr.hpp>

namespace core
{
  void initializePerCPU();

  unsigned cpuidCurrent();
  unsigned getCpusCount();

  template<typename T>
  class PerCPU
  {
  public:
    PerCPU() { m_values = rt::makeUnique<T[]>(getCpusCount()); }

  public:
    const T& get(unsigned cpuid) const { return m_values[cpuid]; }
    T& get(unsigned cpuid)             { return m_values[cpuid]; }

  public:
    const T& current() const { return get(cpuidCurrent()); }
    T& current()             { return get(cpuidCurrent()); }

  private:
    rt::UniquePtr<T[]> m_values;
  };
}

#pragma once

#include <librt/Utility.hpp>
#include <librt/Assert.hpp>

#include <type_traits>

namespace rt
{
  template<typename T>
  struct Global
  {
  private:
#ifndef NDEBUG
    bool m_initialized = false;
    void check(bool state) const { assert(m_initialized == state); }
    void set(bool state) { m_initialized = state; }
#else
    void check(bool state) const {}
    void set(bool state) {}
#endif

  public:
    T& operator()()
    {
      check(true);
      return reinterpret_cast<T&>(m_storage);
    }

    const T& operator()() const
    {
      check(true);
      return reinterpret_cast<T&>(m_storage);
    }

  public:
    template<typename... Args>
    void construct(Args&&... args) { new(&m_storage) T(forward<Args>(args)...); set(true); }
    void destruct() { this->operator()().~T(); set(false); }

  private:
    std::aligned_storage_t<sizeof(T), alignof(T)> m_storage = {};
  };
}

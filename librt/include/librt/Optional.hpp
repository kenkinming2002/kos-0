#pragma once

#include <librt/Utility.hpp>

#include <new>
#include <type_traits>

namespace rt
{
  struct NullOptional {};
  static constexpr auto nullOptional = NullOptional{};

  template<typename T>
  struct Optional
  {
  public:
    bool hasValue () const { return m_initialized; }
    T& get() { return value; }
    const T& get() const { return value; }

  public:
    void reset()
    {
      if(m_initialized)
        value.~T();

      dummy = {};
      m_initialized = false;
    }

  public:
    operator bool() const { return m_initialized; }
    T& operator*() { return get(); }
    const T& operator*() const { get(); }
    T* operator->() { return &get(); }
    const T* operator->() const { return &get(); }

  public:
    Optional(NullOptional) : dummy{}, m_initialized(false) {}
    Optional(T&& t) : value(move(t)), m_initialized(true)  {}
    Optional(const T& t) : value(t), m_initialized(true)  {}
    ~Optional() { reset(); }

  public:
    Optional& operator=(Optional&& other)
    { 
      reset();
      if(other.m_initialized)
      {
        m_initialized = other.m_initialized;
        new(&value) T(move(other.value));
        other.reset();
      }

      return *this;
    }

    Optional& operator=(const Optional& other)
    {
      reset();
      if(other.m_initialized)
      {
        m_initialized = other.m_initialized;
        new(&value) T(other.value);
      }

      return *this;
    }

    Optional(Optional&& other) : Optional(nullOptional) { *this = move(other); }
    Optional(const Optional& other) : Optional(nullOptional) { *this = other; }

  private:
    union
    {
      T value;
      char dummy;
    };
    bool m_initialized;
  };
}

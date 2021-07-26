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
    Optional()             : m_dummy{}, m_initialized(false) {}
    Optional(NullOptional) : m_dummy{}, m_initialized(false) {}
    Optional(T&& t) : m_value(move(t)), m_initialized(true)  {}
    Optional(const T& t) : m_value(t), m_initialized(true)  {}
    ~Optional() { reset(); }

  public:
    Optional& operator=(Optional&& other)
    {
      reset();
      if(other.m_initialized)
      {
        new(&m_value) T(move(other.m_value));
        m_initialized = true;
      }
      return *this;
    }

    Optional& operator=(const Optional& other)
    {
      reset();
      if(other.m_initialized)
      {
        new(&m_value) T(other.m_value);
        m_initialized = true;
      }

      return *this;
    }

    Optional(Optional&& other) : Optional(nullOptional) { *this = move(other); }
    Optional(const Optional& other) : Optional(nullOptional) { *this = other; }

  public:
    void reset()
    {
      if(m_initialized)
        m_value.~T();

      m_dummy = {};
      m_initialized = false;
    }

  public:
    T& get() { return m_value; }
    const T& get() const { return m_value; }

  public:
    bool hasValue () const { return m_initialized; }

  public:
    operator bool() const { return m_initialized; }
    T& operator*() { return get(); }
    const T& operator*() const { return get(); }
    T* operator->() { return &get(); }
    const T* operator->() const { return &get(); }

  public:
    template<typename... Args>
    void emplace(Args&&... args) { *this = Optional(T(forward<Args>()...)); }

  public:
    template<typename Func> auto map(Func func) const & { return hasValue() ? Optional<std::invoke_result_t<Func, const T&>>(func(m_value))       : nullOptional; }
    template<typename Func> auto map(Func func) &&      { return hasValue() ? Optional<std::invoke_result_t<Func, T&&>>(func(move(m_value))) : nullOptional; }

    template<typename U> T valueOr(U&& value) const & { return hasValue() ? m_value       : T(forward<U>(value)); }
    template<typename U> T valueOr(U&& value) &&      { return hasValue() ? move(m_value) : T(forward<U>(value)); }

  private:
    union
    {
      T m_value;
      char m_dummy;
    };
    bool m_initialized;
  };
}

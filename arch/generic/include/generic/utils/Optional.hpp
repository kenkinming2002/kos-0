#pragma once

#include <type_traits>

namespace utils
{
  struct NullOptional
  {
    explicit constexpr NullOptional(int) {}
  };
  constexpr auto nullopt = NullOptional(0);

  //TODO: make this a literal type
  template<typename T>
  struct OptionalBase
  {
  public:
    using value_type = std::decay_t<T>;

    using reference       = value_type&;
    using const_reference = const value_type&;

    using pointer       = value_type*;
    using const_pointer = const value_type*;

  public:
    constexpr OptionalBase() : m_initialized(false) {}
    constexpr OptionalBase(NullOptional) : m_initialized(false) {}

    constexpr OptionalBase(const_reference value) : m_initialized(true)
    {
      this->value() = value;
    }
    constexpr OptionalBase(value_type&& value) : m_initialized(true)
    {
      this->value() = value;
    }

    constexpr OptionalBase(const OptionalBase& other) : m_initialized(true)
    {
      this->value() = other.value();
    }

    constexpr OptionalBase(OptionalBase&& other) : m_initialized(true)
    {
      this->value() = other.value();
    }

  public:
    constexpr const_pointer operator->() const { return &t; }
    constexpr pointer       operator->()       { return &t; }

    constexpr const_reference operator*() const { return t; }
    constexpr reference       operator*()       { return t; }

  public:
    constexpr const_reference value() const { return t; }
    constexpr reference       value()       { return t; }

  public:
    constexpr bool has_value() const { return m_initialized; }
    constexpr operator bool() const { return has_value(); }

  private:
    bool m_initialized;
    struct Empty {};
    union
    {
      Empty empty;
      value_type t;
    };
  };

  template<typename T, typename = void>
  struct Optional : public OptionalBase<T> 
  {
  public:
    using OptionalBase<T>::OptionalBase;

  public:
    ~Optional() { this->value().~value_type(); }
  };

  template<typename T>
  struct Optional<T, std::enable_if_t<std::is_trivially_destructible_v<T>>> : public OptionalBase<T> 
  {
  public:
    using OptionalBase<T>::OptionalBase;
  };
}

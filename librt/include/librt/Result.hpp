#pragma once

#include <librt/Variant.hpp>
#include <librt/Optional.hpp>

namespace rt
{
  template<typename T, typename Error>
  struct Result
  {
  public:
    constexpr Result(T value)     : m_data(type_constant<T>, rt::move(value)) {}
    constexpr Result(Error error) : m_data(type_constant<Error>, rt::move(error)) {}

  public:
    explicit operator bool() const { return m_data.template holds<T>(); }

  public:
    Error error() const         { return m_data.template get<Error>(); }

  public:
    T& operator*()              { return m_data.template get<T>(); }
    const T& operator*() const  { return m_data.template get<T>(); }
    T* operator->()             { return &m_data.template get<T>(); }
    const T* operator->() const { return &m_data.template get<T>(); }

  private:
    Variant<T, Error> m_data;
  };

  template<typename Error>
  struct Result<void, Error>
  {
  public:
    constexpr Result()            : m_data(rt::nullOptional) {}
    constexpr Result(Error error) : m_data(error) {}

  public:
    explicit operator bool() const { return !m_data; }

  public:
    Error error() const         { return *m_data; }

  private:
    Optional<Error> m_data;
  };
}

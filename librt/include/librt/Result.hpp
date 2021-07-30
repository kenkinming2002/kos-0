#pragma once

#include <librt/Variant.hpp>
#include <librt/Optional.hpp>

namespace rt
{
  template<typename T, typename Error>
  struct Result
  {
  public:
    constexpr Result() = delete;
    constexpr Result(T value)     : m_data(type_constant<T>, rt::move(value)) {}
    constexpr Result(Error error) : m_data(type_constant<Error>, rt::move(error)) {}

  public:
    bool ok() const {  return m_data.template holds<T>(); }
    explicit operator bool() const { return ok(); }

  public:
    Error error() const         { return m_data.template get<Error>(); }

  public:
    T& get()              { return m_data.template get<T>(); }
    const T& get() const  { return m_data.template get<T>(); }

  public:
    T& operator*()              { return get(); }
    const T& operator*() const  { return get(); }
    T* operator->()             { return &get(); }
    const T* operator->() const { return &get(); }

  public:
    template<typename Func>
    auto andThen(Func func) &&
    {
      return ok() ? func(rt::move(get())) : error();
    }

    template<typename Func>
    auto map(Func func) && requires std::is_invocable_v<Func, T>
    {
      using U = std::invoke_result_t<Func, T>;
      return ok() ? Result<U, Error>(func(rt::move(get()))) : error();
    }

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
    bool ok() const { return !m_data; }
    explicit operator bool() const { return ok(); }

  public:
    template<typename Func>
    auto andThen(Func func) &&
    {
      return ok() ? func() : error();
    }

    template<typename Func>
    auto map(Func func) && requires std::is_invocable_v<Func>
    {
      using T = std::invoke_result_t<Func>;
      return ok() ? Result<T, Error>(func()) : error();
    }

  public:
    Error error() const         { return *m_data; }

  private:
    Optional<Error> m_data;
  };
}

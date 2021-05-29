#pragma once

#include <type_traits>

namespace rt
{
  template<typename T>
  constexpr std::remove_reference_t<T>&& move(T&& t) { return static_cast<std::remove_reference_t<T>&&>(t); }

  template<typename T>
  constexpr T&& forward(std::remove_reference_t<T>& t) { return static_cast<T&&>(t); }

  template<typename T>
  constexpr T&& forward(std::remove_reference_t<T>&& t) { return static_cast<T&&>(t); }

  template<typename T, typename U = T>
  constexpr T exchange(T& obj, U&& newValue )
  {
    T oldValue = move(obj);
    obj = forward<U>(newValue);
    return oldValue;
  }

  template<typename T>
  constexpr void swap(T& a, T& b)
  {
    T tmp = move(a);
    a = move(b);
    b = move(tmp);
  }

  template<typename T>
  constexpr const T& min(const T& a, const T& b) { return a<b ? a : b; }
  template<typename T>
  constexpr const T& max(const T& a, const T& b) { return a<b ? b : a; }

  template<typename T, typename... Ts>
  constexpr auto min(const T& a, const T& b, const T& c, const Ts&... ts) { return min(min(a, b), c, ts...); }
  template<typename T, typename... Ts>
  constexpr auto max(const T& a, const T& b, const T& c, const Ts&... ts) { return max(max(a, b), c, ts...); }

  template<typename T>
  constexpr const T& asConst(T& t) { return t; }

  template<typename T, typename M>
  constexpr auto memberFunction(T M::* func)
  {
    return [=](auto&& value, auto&&... args){
      return (forward<decltype(value)>(value).*func)(forward<decltype(args)>(args)...);
    };
  }
}

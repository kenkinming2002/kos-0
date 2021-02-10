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
}

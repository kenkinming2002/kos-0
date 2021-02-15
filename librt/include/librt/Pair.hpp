#pragma once

#include <librt/Utility.hpp>

namespace rt
{
  template<typename T1, typename T2>
  struct Pair
  {
    template<typename U, typename V>
    constexpr Pair(U&& u, V&& v) : first(rt::forward<U>(u)), second(rt::forward<V>(v)) {}

    T1 first;
    T2 second;
  };

  template<typename T1, typename T2>
  Pair(T1, T2) -> Pair<T1, T2>;
}

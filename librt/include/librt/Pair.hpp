#pragma once

#include <librt/Utility.hpp>

namespace rt
{
  template<typename T1, typename T2>
  struct Pair
  {
  public:
    constexpr Pair() = default;
    template<typename U, typename V>
    constexpr Pair(Pair<U, V>&& other) : first(move(other.first)), second(move(other.second)) {}
    template<typename U, typename V>
    constexpr Pair& operator=(Pair<U, V>&& other)
    {
      first  = move(other.first);
      second = move(other.second);
    }

  public:
    constexpr Pair(T1 t1, T2 t2) : first(move(t1)), second(move(t2)) {}

  public:
    T1 first;
    T2 second;

    public:
      friend bool operator!=(const Pair& lhs, const Pair& rhs) = default;
      friend bool operator==(const Pair& lhs, const Pair& rhs) = default;
  };

  template<typename T1, typename T2>
  Pair(T1, T2) -> Pair<T1, T2>;
}

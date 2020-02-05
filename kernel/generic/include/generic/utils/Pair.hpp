#pragma once

#include <utils/Utilities.hpp>

namespace utils
{
  template<typename U, typename T>
  struct Pair
  {
  public:
    constexpr Pair() = default;
    constexpr Pair(const U& u, const T& t) : first(u), second(t) {}
    constexpr Pair(U&& u, T&& t) : first(utils::move(u)), second(utils::move(t)) {}

  public:
    constexpr Pair(const Pair&) = default;
    constexpr Pair(Pair&&) = default;

  public:
      U first;
      T second;
  };
}

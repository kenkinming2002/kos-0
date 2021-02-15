#pragma once

#include <stddef.h>

namespace rt
{
  template<typename BidirectionalIterator>
  BidirectionalIterator prev(BidirectionalIterator it)
  {
    --it;
    return it;
  }

  template<typename InputIterator>
  InputIterator next(InputIterator it)
  {
    ++it;
    return it;
  }

  template<typename T, size_t N>
  constexpr T* begin( T(&array)[N] ) { return array; }

  template<typename T, size_t N>
  constexpr T* end( T(&array)[N] ) { return array+N; }
}

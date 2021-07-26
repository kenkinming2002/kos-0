#pragma once

#include <limits>

#include <stdint.h>
#include <stddef.h>

namespace rt::containers
{
  template<size_t N>
  class Bitset
  {
  public:
    static constexpr auto BIT_COUNT = std::numeric_limits<unsigned>::digits;
    static constexpr auto LENGTH    = (N + (BIT_COUNT - 1)) / BIT_COUNT;

  public:
    bool get(size_t n) const
    {
      const auto byteIndex = n / BIT_COUNT;
      const auto bitIndex  = n % BIT_COUNT;
      return m_data[byteIndex] & (1u << bitIndex);
    }

    bool test(size_t n, bool value) const
    {
      return get(n) == value;
    }

    void set(size_t n, bool value)
    {
      const auto byteIndex = n / BIT_COUNT;
      const auto bitIndex  = n % BIT_COUNT;
      m_data[byteIndex] = (m_data[byteIndex] & ~(1u << bitIndex)) | (unsigned(value) << bitIndex);
    }

  private:
    unsigned m_data[LENGTH];
  };
}

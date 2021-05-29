#pragma once

#include <stddef.h>

namespace rt
{
  template<typename T>
  struct Span
  {
  public:
    constexpr Span(T* data = nullptr, size_t length = 0) : m_data(data), m_length(length) {}
    template<size_t N> constexpr Span(T data[N]) : Span(data, N) {}

  public:
    T* begin() { return m_data; }
    T* end()   { return m_data+m_length; }

    const T* begin() const { return m_data; }
    const T* end()   const { return m_data+m_length; }

    const T* cbegin() const { return m_data; }
    const T* cend()   const { return m_data+m_length; }

  private:
    T* m_data;
    size_t m_length;
  };
}

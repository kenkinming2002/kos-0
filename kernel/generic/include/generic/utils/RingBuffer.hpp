#pragma once

#include <stddef.h>

namespace utils
{
  template<typename T, size_t N>
  class RingBuffer
  {
  public:
    void push(const T& data)
    {
      if(!m_full)
      {
        m_buffer[m_writeIndex++] = data;
        m_writeIndex %= N;
        if(m_writeIndex==m_readIndex)
          m_full = true;
      }
      // Silently discard the data
    }

    // NOTE: Behavior is undefined if buffer is empty
    T pop()
    {
      T result = m_buffer[m_readIndex++];
      m_readIndex %= N;
      m_full = false;
      return result;
    }

  public:
    bool empty()
    {
      return (m_writeIndex == m_readIndex) && !m_full;
    }

    bool full()
    {
      return m_full;
    }

  private:
    T m_buffer[N];
    size_t m_writeIndex = 0, m_readIndex = 0;
    bool m_full = false;
  };
}

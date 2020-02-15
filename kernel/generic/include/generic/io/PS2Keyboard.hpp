#pragma once

#include <generic/utils/RingBuffer.hpp>

#include <stdint.h>
#include <optional>

namespace io
{
  class PS2Keyboard
  {
  public:
    PS2Keyboard();

  public:
    void push(uint8_t scanCode);

  public:
    std::optional<uint8_t> poll();

  private:
    utils::RingBuffer<uint8_t, 64> m_buffer;
  };

  extern PS2Keyboard ps2Keyboard;
}

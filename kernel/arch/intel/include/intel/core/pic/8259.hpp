#pragma once

#include <core/Interrupt.hpp>

#include <stdint.h>

namespace core::pic
{
  class Controller8259
  {
  public:
    constexpr static uint8_t DEFAULT_MASTER_OFFSET = 0x20;
    constexpr static uint8_t DEFAULT_SLAVE_OFFSET = 0x28;

  public:
    Controller8259(uint8_t masterOffset = DEFAULT_MASTER_OFFSET, uint8_t slaveOffset = DEFAULT_SLAVE_OFFSET);

  public:
    void remap(uint8_t masterOffset, uint8_t slaveOffset) const;
    void acknowledge(uint8_t interrupt) const;
  };
}

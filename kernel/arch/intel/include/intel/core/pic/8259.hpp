#pragma once

#include <core/Interrupt.hpp>

#include <stdint.h>

namespace core::pic::controller8259
{
  constexpr static uint8_t DEFAULT_MASTER_OFFSET = 0x20;
  constexpr static uint8_t DEFAULT_SLAVE_OFFSET = 0x28;

  int init();
  void setMask(uint8_t irqLine);
  void clearMask(uint8_t irqLine);
  void acknowledge(uint8_t interrupt);
}

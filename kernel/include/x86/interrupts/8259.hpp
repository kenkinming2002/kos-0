#pragma once

#include <stdint.h>

namespace core::interrupts
{
  constexpr static uint8_t PIC_OFFSET = 0x20;

  void initialize8259();

  void setMask(uint8_t irqLine);
  void clearMask(uint8_t irqLine);
  void acknowledge(uint8_t interrupt);
}

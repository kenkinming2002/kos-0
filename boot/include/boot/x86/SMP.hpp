#pragma once

#include <stdint.h>

namespace boot
{
  void initializeSMP();
  void apContinue();

  uint8_t getCoresCount();
}

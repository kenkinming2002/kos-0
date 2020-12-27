#pragma once

#include <stdint.h>

namespace core::assembly
{
  void wrmsr(uint32_t reg, uint64_t value);
  uint64_t rdmsr(uint32_t reg);
}

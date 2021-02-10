#pragma once

#include <stdint.h>

namespace core::internals
{
  void initializeSegmentation();
  void setKernelStack(uint32_t ss, uint32_t esp);
}

#pragma once

#include <i686/internals/GDT.hpp>
#include <i686/internals/TSS.hpp>

namespace core::internals
{
  void initializeSegmentation();
  void setKernelStack(uint32_t ss, uint32_t esp);
}

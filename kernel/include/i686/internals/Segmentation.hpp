#pragma once

#include <i686/internals/GDT.hpp>
#include <i686/internals/TSS.hpp>

namespace core::internals
{
  extern TSS tss;
  void initializeSegmentation();
}

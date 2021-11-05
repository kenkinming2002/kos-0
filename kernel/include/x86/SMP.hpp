#pragma once

#include <generic/BootInformation.hpp>

namespace core
{
  // Only return for BSP
  void initializeSMP(BootInformation* bootInformation, unsigned apicid);
}

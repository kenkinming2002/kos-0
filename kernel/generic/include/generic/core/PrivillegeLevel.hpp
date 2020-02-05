#pragma once

#include <stdint.h>

namespace core
{
  enum class PrivillegeLevel : uint8_t
  {
    RING0 = 0,
    RING1 = 1,
    RING2 = 2,
    RING3 = 3
  };
}

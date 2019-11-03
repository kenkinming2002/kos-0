#pragma once

#include <generic/core/PrivillegeLevel.hpp>

#include <stddef.h>

/** @file core/Interrupt.h
 *  
 *  Generic Interface for Interrupt Handling
 */

namespace core
{
  using Handler = void(*)();
}

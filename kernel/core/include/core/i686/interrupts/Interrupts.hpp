#pragma once

#include <core/i686/PrivilegeLevel.hpp>

#include <stddef.h>
#include <stdint.h>

namespace core::interrupts
{
  void setKernelStack(uintptr_t ptr, size_t size);

  typedef void(*Handler)(uint8_t irqNumber, uint32_t errorCode, uintptr_t oldEip);
  void installHandler(int irqNumber, Handler handler, PrivilegeLevel privilegeLevel, bool disableInterrupt);
  void uninstallHandler(int irqNumber);
}

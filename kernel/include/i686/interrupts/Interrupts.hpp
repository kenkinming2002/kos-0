#pragma once

#include <sys/Types.hpp>

#include <i686/PrivilegeLevel.hpp>

#include <stddef.h>
#include <stdint.h>

namespace core::interrupts
{
  using Handler =  void(*)(irq_t irqNumber, uword_t errorCode, uintptr_t oldEip);

  void initialize();

  void setKernelStack(uintptr_t ptr, size_t size);

  void installHandler(int irqNumber, Handler handler, PrivilegeLevel privilegeLevel, bool disableInterrupt);
  void uninstallHandler(int irqNumber);
}

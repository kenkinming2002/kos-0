#pragma once

#include <stdint.h>

namespace core
{
  void init_syscall();
  void set_syscall_esp(uintptr_t esp);
}

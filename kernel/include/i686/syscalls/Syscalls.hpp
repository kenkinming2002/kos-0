#pragma once

#include <stddef.h>
#include <stdint.h>

namespace core::syscalls
{
  void initialize();
  void setKernelStack(uintptr_t ptr, size_t size);

  typedef int(*Handler)(int syscallNumber, int a1, int a2, int a3);
  static constexpr int MAX_SYSCALL_COUNT = 256;

  void installHandler(int syscallNumber, Handler handler);
  void uninstallHandler(int syscallNumber);
}

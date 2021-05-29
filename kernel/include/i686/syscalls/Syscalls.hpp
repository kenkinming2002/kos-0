#pragma once

#include <limits>
#include <stddef.h>
#include <stdint.h>

namespace core::syscalls
{
  void initialize();
  void setKernelStack(uintptr_t ptr, size_t size);

  /* We have maximum of 6 arguments, this may expand? ;) */
  typedef int(*Handler)(int syscallNumber, int a1, int a2, int a3, int a4, int a5, int a6);
  static constexpr int MAX_SYSCALL_COUNT = 256;

  void installHandler(int syscallNumber, Handler handler);
  void uninstallHandler(int syscallNumber);
}

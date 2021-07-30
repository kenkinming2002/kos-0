#pragma once

#include <sys/Types.hpp>

enum Syscalls : uword_t
{
  // Filesystem management
  SYS_ASYNC_SUBMIT = 0,
  SYS_ASYNC_WAIT   = 1,

  // Memory management
  SYS_MMAP     = 2,
  SYS_MUNMAP   = 3,
  SYS_MREMAP   = 4,

  // Process management
  SYS_GETPID   = 5,
  SYS_YIELD    = 6,
  SYS_KILL     = 7,
  SYS_FORK     = 8,

  // Misc
  SYS_TEST     = 9,
  SYS_LOG      = 10
};


#pragma once

#include <sys/Types.hpp>

enum Syscalls : uword_t
{
  // Filesystem management
  SYS_ROOT     = 0,
  SYS_MOUNTAT  = 1,
  SYS_UMOUNTAT = 2,
  SYS_OPENAT   = 3,
  SYS_CREATEAT = 4,
  SYS_LINKAT   = 5,
  SYS_UNLINKAT = 6,
  SYS_READDIR  = 7,
  SYS_SEEK     = 8,
  SYS_READ     = 9,
  SYS_WRITE    = 10,
  SYS_RESIZE   = 11,
  SYS_CLOSE    = 12,

  // Memory management
  SYS_MMAP     = 13,
  SYS_MUNMAP   = 14,
  SYS_MREMAP   = 15,

  // Process management
  SYS_GETPID   = 16,
  SYS_YIELD    = 17,
  SYS_KILL     = 18,

  // Misc
  SYS_TEST     = 19,
  SYS_LOG      = 20
};


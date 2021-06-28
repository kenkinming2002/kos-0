#pragma once

#include <sys/Types.hpp>

enum Syscalls : uword_t
{
  SYS_YIELD    = 0,
  SYS_ROOT     = 1,
  SYS_MOUNTAT  = 2,
  SYS_UMOUNTAT = 3,
  SYS_OPENAT   = 4,
  SYS_CREATEAT = 5,
  SYS_LINKAT   = 6,
  SYS_UNLINKAT = 7,
  SYS_READDIR  = 8,
  SYS_SEEK     = 9,
  SYS_READ     = 10,
  SYS_WRITE    = 11,
  SYS_RESIZE   = 12,
  SYS_CLOSE    = 13,

  SYS_MMAP     = 14,
  SYS_MUNMAP   = 15,
  SYS_MREMAP   = 16,

  SYS_TEST     = 17,
  SYS_LOG      = 18
};


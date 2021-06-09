#pragma once

#include <generic/Types.hpp>
#include <generic/Error.hpp>

#include <librt/Algorithm.hpp>
#include <librt/Optional.hpp>
#include <librt/String.hpp>
#include <librt/Cast.hpp>

#include <limits>
#include <stddef.h>
#include <stdint.h>

namespace core::syscalls
{
  enum Syscalls
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

    SYS_TEST     = 14,
    SYS_LOG      = 15
  };

  void initialize();
  void setKernelStack(uintptr_t ptr, size_t size);

  /* There is a problem, and that is that I would like not take int as paramters
   * to syscalls, but rather something more legible like const char* or size_t
   * or ssize_t */

  // TODO: Put this into a separate file

  /* We have maximum of 6 arguments, this may expand? ;) */
  static constexpr int MAX_SYSCALL_COUNT = 256;
  typedef uword_t(*Handler)(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6);

  void installHandler(int syscallNumber, Handler handler);
  void uninstallHandler(int syscallNumber);
}

#define WRAP_SYSCALL0(name, realName) \
  core::uword_t name(core::uword_t a1, core::uword_t a2, core::uword_t a3, core::uword_t a4, core::uword_t a5, core::uword_t a6) \
  { \
    return rt::bitCast<core::uword_t>(realName( \
    )); \
  }

#define WRAP_SYSCALL1(name, realName) \
  core::uword_t name(core::uword_t a1, core::uword_t a2, core::uword_t a3, core::uword_t a4, core::uword_t a5, core::uword_t a6) \
  { \
    return rt::bitCast<core::uword_t>(realName( \
      rt::autoBitCast(a1) \
    )); \
  }

#define WRAP_SYSCALL2(name, realName) \
  core::uword_t name(core::uword_t a1, core::uword_t a2, core::uword_t a3, core::uword_t a4, core::uword_t a5, core::uword_t a6) \
  { \
    return rt::bitCast<core::uword_t>(realName( \
      rt::autoBitCast(a1), \
      rt::autoBitCast(a2) \
    )); \
  }

#define WRAP_SYSCALL3(name, realName) \
  core::uword_t name(core::uword_t a1, core::uword_t a2, core::uword_t a3, core::uword_t a4, core::uword_t a5, core::uword_t a6) \
  { \
    return rt::bitCast<core::uword_t>(realName( \
      rt::autoBitCast(a1), \
      rt::autoBitCast(a2), \
      rt::autoBitCast(a3) \
    )); \
  }

#define WRAP_SYSCALL4(name, realName) \
  core::uword_t name(core::uword_t a1, core::uword_t a2, core::uword_t a3, core::uword_t a4, core::uword_t a5, core::uword_t a6) \
  { \
    return rt::bitCast<core::uword_t>(realName( \
      rt::autoBitCast(a1), \
      rt::autoBitCast(a2), \
      rt::autoBitCast(a3), \
      rt::autoBitCast(a4) \
    )); \
  }

#define WRAP_SYSCALL5(name, realName) \
  core::uword_t name(core::uword_t a1, core::uword_t a2, core::uword_t a3, core::uword_t a4, core::uword_t a5, core::uword_t a6) \
  { \
    return rt::bitCast<core::uword_t>(realName( \
      rt::autoBitCast(a1), \
      rt::autoBitCast(a2), \
      rt::autoBitCast(a3), \
      rt::autoBitCast(a4), \
      rt::autoBitCast(a5) \
    )); \
  }

#define WRAP_SYSCALL6(name, realName) \
  core::uword_t name(core::uword_t a1, core::uword_t a2, core::uword_t a3, core::uword_t a4, core::uword_t a5, core::uword_t a6) \
  { \
    return rt::bitCast<core::uword_t>(realName( \
      rt::autoBitCast(a1), \
      rt::autoBitCast(a2), \
      rt::autoBitCast(a3), \
      rt::autoBitCast(a4), \
      rt::autoBitCast(a5), \
      rt::autoBitCast(a6)  \
    )); \
  }



#pragma once

#include <sys/Types.hpp>
#include <sys/Syscalls.hpp>

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
  void initialize();
  void setKernelStack(uintptr_t ptr, size_t size);

  /* There is a problem, and that is that I would like not take int as paramters
   * to syscalls, but rather something more legible like const char* or size_t
   * or ssize_t */

  // TODO: Put this into a separate file

  /* We have maximum of 6 arguments, this may expand? ;) */
  static constexpr int MAX_SYSCALL_COUNT = 256;
  typedef Result<result_t>(*Handler)(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6);

  void installHandler(int syscallNumber, Handler handler);
  void uninstallHandler(int syscallNumber);
}

#define UNWRAP(v) do { if(!(v)) return v.error(); } while(0)

#define WRAP_SYSCALL0(name, realName) \
  core::Result<result_t> name(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6) \
  { \
    return realName(); \
  }

#define WRAP_SYSCALL1(name, realName) \
  core::Result<result_t> name(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6) \
  { \
    return realName(rt::autoBitCast(a1)); \
  }

#define WRAP_SYSCALL2(name, realName) \
  core::Result<result_t> name(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6) \
  { \
    return realName(rt::autoBitCast(a1), rt::autoBitCast(a2)); \
  }

#define WRAP_SYSCALL3(name, realName) \
  core::Result<result_t> name(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6) \
  { \
    return realName(rt::autoBitCast(a1), rt::autoBitCast(a2), rt::autoBitCast(a3)); \
  }

#define WRAP_SYSCALL4(name, realName) \
  core::Result<result_t> name(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6) \
  { \
    return realName(rt::autoBitCast(a1), rt::autoBitCast(a2), rt::autoBitCast(a3), rt::autoBitCast(a4)); \
  }

#define WRAP_SYSCALL5(name, realName) \
  core::Result<result_t> name(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6) \
  { \
    return realName(rt::autoBitCast(a1), rt::autoBitCast(a2), rt::autoBitCast(a3), rt::autoBitCast(a4), rt::autoBitCast(a5)); \
  }

#define WRAP_SYSCALL6(name, realName) \
  core::Result<result_t> name(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6) \
  { \
    return realName(rt::autoBitCast(a1), rt::autoBitCast(a2), rt::autoBitCast(a3), rt::autoBitCast(a4), rt::autoBitCast(a5), rt::autoBitCast(a6)); \
  }



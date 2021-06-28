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
  typedef result_t(*Handler)(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6);

  void installHandler(int syscallNumber, Handler handler);
  void uninstallHandler(int syscallNumber);

  inline int makeError(ErrorCode errorCode) { return -static_cast<int>(errorCode); }
}

#define UNWRAP(v) do { if(!(v)) return v.error(); } while(0)

#define WRAP_SYSCALL0(name, realName) \
  result_t name(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6) \
  { \
    auto result = realName(); \
    return result ? rt::bitCast<word_t>(*result) : core::syscalls::makeError(result.error()); \
  }

#define WRAP_SYSCALL1(name, realName) \
  result_t name(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6) \
  { \
    auto result = realName(rt::autoBitCast(a1)); \
    return result ? rt::bitCast<word_t>(*result) : core::syscalls::makeError(result.error()); \
  }

#define WRAP_SYSCALL2(name, realName) \
  result_t name(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6) \
  { \
    auto result = realName(rt::autoBitCast(a1), rt::autoBitCast(a2)); \
    return result ? rt::bitCast<word_t>(*result) : core::syscalls::makeError(result.error()); \
  }

#define WRAP_SYSCALL3(name, realName) \
  result_t name(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6) \
  { \
    auto result = realName(rt::autoBitCast(a1), rt::autoBitCast(a2), rt::autoBitCast(a3)); \
    return result ? rt::bitCast<word_t>(*result) : core::syscalls::makeError(result.error()); \
  }

#define WRAP_SYSCALL4(name, realName) \
  result_t name(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6) \
  { \
    auto result = realName(rt::autoBitCast(a1), rt::autoBitCast(a2), rt::autoBitCast(a3), rt::autoBitCast(a4)); \
    return result ? rt::bitCast<word_t>(*result) : core::syscalls::makeError(result.error()); \
  }

#define WRAP_SYSCALL5(name, realName) \
  result_t name(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6) \
  { \
    auto result = realName(rt::autoBitCast(a1), rt::autoBitCast(a2), rt::autoBitCast(a3), rt::autoBitCast(a4), rt::autoBitCast(a5)); \
    return result ? rt::bitCast<word_t>(*result) : core::syscalls::makeError(result.error()); \
  }

#define WRAP_SYSCALL6(name, realName) \
  result_t name(uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6) \
  { \
    auto result = realName(rt::autoBitCast(a1), rt::autoBitCast(a2), rt::autoBitCast(a3), rt::autoBitCast(a4), rt::autoBitCast(a5), rt::autoBitCast(a6)); \
    return result ? rt::bitCast<word_t>(*result) : core::syscalls::makeError(result.error()); \
  }



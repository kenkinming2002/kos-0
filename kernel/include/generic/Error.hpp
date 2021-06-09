#pragma once

#include <librt/Result.hpp>

namespace core
{
  enum class ErrorCode
  {
    INVALID=1,
    EXIST,
    NOT_EXIST,
    BUSY,
    OUT_OF_MEMORY,
    UNSUPPORTED,
    NFILE,
    BADFD,
    FAULT,
    OVERFLOW,
    OTHER
  };

  template<typename T>
  using Result = rt::Result<T, ErrorCode>;
}

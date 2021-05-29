#pragma once

#include <librt/Result.hpp>
namespace core::vfs
{
  enum class ErrorCode { INVALID, EXIST, NOT_EXIST, BUSY, OUT_OF_MEMORY, UNSUPPORTED, OTHER };
  template<typename T>
  using Result = rt::Result<T, ErrorCode>;
}

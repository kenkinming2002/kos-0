#pragma once

#include <sys/Error.hpp>

#include <librt/Result.hpp>

namespace core
{
  template<typename T>
  using Result = rt::Result<T, ErrorCode>;
}

#pragma once

#include <generic/vfs/Error.hpp>

#include <librt/SharedPtr.hpp>

namespace core::vfs
{
  class SuperBlock : public rt::SharedPtrHook
  {
  public:
    virtual ~SuperBlock() = default;

  public:
    struct Stat { }; // We may not even be from a filesystem
    virtual Result<Stat> stat() { return ErrorCode::UNSUPPORTED; }
  };
}


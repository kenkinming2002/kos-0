#pragma once

#include <generic/vfs/Inode.hpp>
#include <generic/Error.hpp>

#include <librt/SharedPtr.hpp>
#include <librt/Result.hpp>
#include <librt/Span.hpp>
#include <librt/StringRef.hpp>
#include <librt/UniquePtr.hpp>

#include <librt/containers/List.hpp>

namespace core::vfs
{
  class Mountable : public rt::containers::ListHook
  {
  public:
    virtual Result<rt::SharedPtr<Inode>> mount(rt::StringRef arg) = 0;
    virtual rt::StringRef name() = 0;

  public:
    virtual ~Mountable() = default;
  };

  void initializeMountables();

  void registerMountable(Mountable& mountable);
  void deregisterMountable(Mountable& mountable);

  Mountable* lookupMountable(rt::StringRef name);
}

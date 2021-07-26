#pragma once

#include <generic/vfs/Inode.hpp>
#include <generic/Error.hpp>

#include <librt/containers/IntrusiveList.hpp>

#include <librt/Result.hpp>
#include <librt/StringRef.hpp>


namespace core::vfs
{
  class Mountable : public rt::containers::IntrusiveListHook
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

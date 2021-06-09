#pragma once

#include <generic/vfs/Mountable.hpp>
#include <generic/vfs/Vnode.hpp>
#include <generic/vfs/File.hpp>
#include <generic/Error.hpp>

#include <librt/Result.hpp>
#include <librt/SharedPtr.hpp>
#include <librt/String.hpp>
#include <librt/UniquePtr.hpp>
#include <librt/StringRef.hpp>
#include <librt/Span.hpp>
#include <librt/containers/List.hpp>
#include <librt/containers/Map.hpp>

#include <stddef.h>

namespace core::vfs
{
  void initialize();

  File root();

  Result<void> mountAt(File& at, rt::StringRef mountpoint, rt::StringRef mountableName, rt::StringRef arg);
  Result<void> mountAt(File& at, rt::StringRef mountpoint, Mountable& mountable, rt::StringRef arg);
  Result<void> umountAt(File& at, rt::StringRef mountpoint);

  Result<File> openAt(File& at, rt::StringRef path);
  Result<File> createAt(File& at, rt::StringRef path, Type type);
  Result<void> linkAt(File& at, rt::StringRef path, rt::StringRef target);
  Result<void> unlinkAt(File& at, rt::StringRef path);
}

#pragma once

#include <generic/vfs/File.hpp>
#include <generic/Error.hpp>

#include <librt/SharedPtr.hpp>
#include <librt/StringRef.hpp>

namespace core::vfs
{
  class Mountable;

  void initialize();

  rt::SharedPtr<File> root();

  Result<void> mountAt(rt::SharedPtr<File> at, rt::StringRef mountpoint, rt::StringRef mountableName, rt::StringRef arg);
  Result<void> mountAt(rt::SharedPtr<File> at, rt::StringRef mountpoint, Mountable& mountable, rt::StringRef arg);
  Result<void> umountAt(rt::SharedPtr<File> at, rt::StringRef mountpoint);

  Result<rt::SharedPtr<File>> openAt(rt::SharedPtr<File> at, rt::StringRef path);
  Result<rt::SharedPtr<File>> createAt(rt::SharedPtr<File> at, rt::StringRef path, Type type);
  Result<void> linkAt(rt::SharedPtr<File> at, rt::StringRef path, rt::StringRef target);
  Result<void> unlinkAt(rt::SharedPtr<File> at, rt::StringRef path);
}

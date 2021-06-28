#pragma once

#include "librt/SharedPtr.hpp"
#include <generic/vfs/File.hpp>

#include <librt/containers/StaticVector.hpp>

#include <atomic>
#include <type_traits>

namespace core::vfs
{
  static constexpr size_t MAX_FD = 16;
  class FileDescriptors
  {
  public:
    Result<fd_t> addFile(rt::SharedPtr<File> file);
    Result<rt::SharedPtr<File>> getFile(fd_t fd);
    Result<void> removeFile(fd_t fd);

  private:
    rt::SharedPtr<File> m_files[MAX_FD];
  };
}

#pragma once

#include <generic/vfs/File.hpp>

#include <librt/SharedPtr.hpp>
#include <librt/SpinLock.hpp>

namespace core::vfs
{
  static constexpr size_t MAX_FD = 16;
  class FileDescriptors
  {
  public:
    constexpr FileDescriptors() = default;
    FileDescriptors(const FileDescriptors& other);
    FileDescriptors& operator=(const FileDescriptors& other);

  public:
    Result<fd_t> addFile(rt::SharedPtr<File> file);
    Result<rt::SharedPtr<File>> getFile(fd_t fd);
    Result<void> removeFile(fd_t fd);

  private:
    rt::SpinLock m_lock;
    rt::SharedPtr<File> m_files[MAX_FD];
  };
}

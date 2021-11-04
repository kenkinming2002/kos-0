#pragma once

#include <generic/vfs/File.hpp>

#include <librt/SharedPtr.hpp>
#include <generic/SpinLock.hpp>

namespace core::vfs
{
  static constexpr size_t MAX_FD = 16;
  class FileDescriptors : public rt::SharedPtrHook
  {
  public:
    rt::SharedPtr<FileDescriptors> clone();

  public:
    Result<fd_t> addFile(rt::SharedPtr<File> file);
    Result<rt::SharedPtr<File>> getFile(fd_t fd);
    Result<void> removeFile(fd_t fd);

  private:
    core::SpinLock m_lock;
    rt::SharedPtr<File> m_files[MAX_FD];
  };
}

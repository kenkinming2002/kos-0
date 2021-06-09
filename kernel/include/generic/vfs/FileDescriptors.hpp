#pragma once

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
    Result<int> addFile(vfs::File file);
    Result<File> getFile(int fd);
    Result<void> removeFile(int fd);

  private:
    rt::containers::StaticVector<File, MAX_FD> m_files;
  };
}

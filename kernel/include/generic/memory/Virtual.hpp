#pragma once

#include <generic/memory/Memory.hpp>
#include <generic/vfs/File.hpp>

namespace core::memory
{
  void initializeVirtual();

  Result<void> map(uintptr_t addr, size_t length, Prot prot, rt::SharedPtr<vfs::File> file, size_t fileOffset, size_t fileLength);
  Result<void> unmap(uintptr_t addr, size_t length);
  Result<void> remap(uintptr_t addr, size_t length, size_t newLength);
}

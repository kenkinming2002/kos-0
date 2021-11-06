#include <generic/memory/Virtual.hpp>

#include <generic/memory/Syscalls.hpp>
#include <generic/tasks/Scheduler.hpp>

#include <i686/syscalls/Access.hpp>

namespace core::memory
{
  void initializeVirtual()
  {
    MemoryMapping::initialize();
    initializeSyscalls();
  }

  Result<void> map(uintptr_t addr, size_t length, Prot prot, rt::SharedPtr<vfs::File> file, size_t fileOffset, size_t fileLength)
  {
    if(!syscalls::verifyRegionUser(addr, length))
      return ErrorCode::FAULT;

    auto memoryMapping = tasks::current().memoryMapping;
    return memoryMapping->map(addr, length, prot, rt::move(file), fileOffset, fileLength);
  }

  Result<void> unmap(uintptr_t addr, size_t length)
  {
    if(!syscalls::verifyRegionUser(addr, length))
      return ErrorCode::FAULT;

    auto memoryMapping = tasks::current().memoryMapping;
    return memoryMapping->unmap(addr, length);
  }

  Result<void> remap(uintptr_t addr, size_t length, size_t newLength)
  {
    if(!syscalls::verifyRegionUser(addr, length))
      return ErrorCode::FAULT;

    if(!syscalls::verifyRegionUser(addr, newLength))
      return ErrorCode::FAULT;

    auto memoryMapping = tasks::current().memoryMapping;
    return memoryMapping->remap(addr, length, newLength);
  }
}

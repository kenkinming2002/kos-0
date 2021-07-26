#include <generic/memory/Virtual.hpp>

#include <i686/tasks/Task.hpp>
#include <i686/syscalls/Syscalls.hpp>
#include <i686/syscalls/Access.hpp>

namespace core::memory
{
  void initializeVirtual()
  {
    MemoryMapping::initialize();
  }

  Result<void> map(uintptr_t addr, size_t length, Prot prot, rt::SharedPtr<vfs::File> file, size_t fileOffset, size_t fileLength)
  {
    if(!syscalls::verifyRegionUser(addr, length))
      return ErrorCode::FAULT;

    auto task = tasks::Task::current();
    auto memoryMapping = task->memoryMapping;

    return memoryMapping->map(addr, length, prot, rt::move(file), fileOffset, fileLength);
  }

  Result<void> unmap(uintptr_t addr, size_t length)
  {
    if(!syscalls::verifyRegionUser(addr, length))
      return ErrorCode::FAULT;

    auto task = tasks::Task::current();
    auto memoryMapping = task->memoryMapping;

    return memoryMapping->unmap(addr, length);
  }

  Result<void> remap(uintptr_t addr, size_t length, size_t newLength)
  {
    if(!syscalls::verifyRegionUser(addr, length))
      return ErrorCode::FAULT;

    if(!syscalls::verifyRegionUser(addr, newLength))
      return ErrorCode::FAULT;

    auto task = tasks::Task::current();
    auto memoryMapping = task->memoryMapping;

    return memoryMapping->remap(addr, length, newLength);
  }

  uintptr_t kmap(physaddr_t phyaddr)
  {
    return MemoryMapping::current()->kmap(phyaddr);
  }

  void kunmap(uintptr_t addr)
  {
    return MemoryMapping::current()->kunmap(addr);
  }
}

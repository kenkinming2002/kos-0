#include <generic/memory/Syscalls.hpp>

#include <generic/memory/Memory.hpp>

#include <i686/tasks/Task.hpp>
#include <i686/syscalls/Syscalls.hpp>
#include <i686/syscalls/Access.hpp>

namespace core::memory
{
  namespace
  {
    Result<result_t> sys_mmap(uintptr_t addr, size_t length, Prot prot, fd_t fd, size_t offset)
    {
      if(!syscalls::verifyRegionUser(addr, length))
        return syscalls::makeError(ErrorCode::FAULT);

      auto task = tasks::Task::current();
      auto memoryMapping = task->memoryMapping;

      auto file = fd != FD_NONE ? task->fileDescriptors.getFile(fd) : rt::SharedPtr<vfs::File>(nullptr);
      UNWRAP(file);

      auto result = memoryMapping->map(addr, length, prot, rt::move(*file), offset);
      UNWRAP(result);

      return 0;
    }
    WRAP_SYSCALL5(_sys_mmap, sys_mmap)

    Result<result_t> sys_munmap(uintptr_t addr, size_t length)
    {
      if(!syscalls::verifyRegionUser(addr, length))
        return syscalls::makeError(ErrorCode::FAULT);

      auto task = tasks::Task::current();
      auto memoryMapping = task->memoryMapping;

      auto result = memoryMapping->unmap(addr, length);
      UNWRAP(result);

      return 0;
    }
    WRAP_SYSCALL2(_sys_munmap, sys_munmap)

    Result<result_t> sys_mremap(uintptr_t addr, size_t length, size_t newLength)
    {
      if(!syscalls::verifyRegionUser(addr, length))
        return syscalls::makeError(ErrorCode::FAULT);

      if(!syscalls::verifyRegionUser(addr, newLength))
        return syscalls::makeError(ErrorCode::FAULT);

      auto task = tasks::Task::current();
      auto memoryMapping = task->memoryMapping;

      auto result = memoryMapping->remap(addr, length, newLength);
      UNWRAP(result);

      return 0;
    }
    WRAP_SYSCALL3(_sys_mremap, sys_mremap)
  }

  void initializeSyscalls()
  {
    syscalls::installHandler(SYS_MMAP,   &_sys_mmap);
    syscalls::installHandler(SYS_MUNMAP, &_sys_munmap);
    syscalls::installHandler(SYS_MREMAP, &_sys_mremap);
  }
}

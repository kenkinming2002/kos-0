#include <generic/memory/Syscalls.hpp>

#include <generic/tasks/Scheduler.hpp>

#include <i686/syscalls/Syscalls.hpp>
#include <i686/syscalls/Access.hpp>

#include <generic/memory/Virtual.hpp>

namespace core::memory
{
  namespace
  {
    Result<result_t> sys_mmap(uintptr_t addr, size_t length, Prot prot, fd_t fd, size_t fileOffset, size_t fileLength)
    {
      auto file = fd != FD_NONE ? tasks::current().fileDescriptors->getFile(fd) : rt::SharedPtr<vfs::File>(nullptr);
      UNWRAP(file);
      auto result = map(addr, length, prot, rt::move(*file), fileOffset, fileLength);
      UNWRAP(result);
      return 0;
    }
    WRAP_SYSCALL6(_sys_mmap, sys_mmap)

    Result<result_t> sys_munmap(uintptr_t addr, size_t length)
    {
      auto result = unmap(addr, length);
      UNWRAP(result);
      return 0;
    }
    WRAP_SYSCALL2(_sys_munmap, sys_munmap)

    Result<result_t> sys_mremap(uintptr_t addr, size_t length, size_t newLength)
    {
      auto result = remap(addr, length, newLength);
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

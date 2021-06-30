#include <generic/tasks/Syscalls.hpp>

#include <generic/tasks/Scheduler.hpp>

#include <i686/syscalls/Syscalls.hpp>

namespace core::tasks
{
  namespace
  {
    static Result<pid_t> sys_getpid()
    {
      return Task::current()->pid;
    }
    WRAP_SYSCALL0(_sys_getpid, sys_getpid)

    static Result<result_t> sys_yield()
    {
      schedule();
      return 0;
    }
    WRAP_SYSCALL0(_sys_yield, sys_yield)

    static Result<result_t> sys_kill(pid_t pid, status_t status)
    {
      return kill(pid, status);
    }
    WRAP_SYSCALL2(_sys_kill, sys_kill)
  }

  void initializeSyscalls()
  {
    syscalls::installHandler(SYS_GETPID, &_sys_getpid);
    syscalls::installHandler(SYS_YIELD,  &_sys_yield);
    syscalls::installHandler(SYS_KILL,   &_sys_kill);
  }
}

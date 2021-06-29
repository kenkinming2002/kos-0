#include "librt/SharedPtr.hpp"
#include <generic/tasks/Scheduler.hpp>

#include <i686/syscalls/Syscalls.hpp>
#include <i686/interrupts/Interrupts.hpp>
#include <i686/tasks/Switch.hpp>

#include <x86/interrupts/8259.hpp>

#include <librt/Panic.hpp>
#include <librt/Global.hpp>
#include <librt/Iterator.hpp>
#include <librt/Log.hpp>
#include <librt/Assert.hpp>

namespace core::tasks
{
  namespace
  {
    static Result<result_t> sys_yield()
    {
      schedule();
      return 0;
    }
    WRAP_SYSCALL0(_sys_yield, sys_yield)

    static Result<result_t> sys_exit(status_t status)
    {
      pid_t pid = Task::current()->pid;
      kill(pid, status);
      schedule();
      __builtin_unreachable();
    }
    WRAP_SYSCALL1(_sys_exit, sys_exit)

    static Result<result_t> sys_kill(pid_t pid, status_t status)
    {
      return kill(pid, status);
    }
    WRAP_SYSCALL2(_sys_kill, sys_kill)

    /* Note: We have to disable interrupt, however, locking would not be
     *       necessary, since we would or *WILL* have a per-cpu task queue.  */
    void timerHandler(uint8_t, uint32_t, uintptr_t)
    {
      interrupts::acknowledge(0);
      schedule();
    }
  }

  namespace
  {
    constinit rt::Global<rt::containers::List<rt::SharedPtr<Task>>> activeTasksList;
    constinit rt::Global<rt::containers::List<rt::SharedPtr<Task>>> terminatedTasksList;
  }

  namespace
  {
    void reaper()
    {
      for(;;)
      {
        if(!terminatedTasksList().empty())
        {
          auto task = *terminatedTasksList().begin();
          terminatedTasksList().erase(terminatedTasksList().begin());
          rt::logf("Reaping process pid = %ld, status = %ld\n", task->pid, task->status);
        }

        schedule();
      }
      ASSERT_UNREACHABLE;
    }
  }

  void initializeScheduler()
  {
    activeTasksList.construct();
    terminatedTasksList.construct();

    syscalls::installHandler(SYS_YIELD, &_sys_yield);
    syscalls::installHandler(SYS_KILL,  &_sys_kill);
    syscalls::installHandler(SYS_EXIT,  &_sys_exit);

    interrupts::installHandler(0x20, &timerHandler, PrivilegeLevel::RING0, true);
    interrupts::clearMask(0);

    auto task = addTask();
    task->asKernelTask(&reaper);
  }

  rt::SharedPtr<Task> addTask()
  {
    auto task = Task::allocate();
    if(!task)
      return nullptr;

    auto it = activeTasksList().insert(activeTasksList().end(), rt::move(task));
    return *it;
  }

  namespace
  {
    rt::SharedPtr<Task> getNextTask()
    {
      if(activeTasksList().empty())
        return nullptr;

      auto nextTask = *activeTasksList().begin();
      activeTasksList().erase(activeTasksList().begin());
      activeTasksList().insert(activeTasksList().end(), nextTask);
      return nextTask;
    }
  }

  void schedule()
  {
    auto currentTask = Task::current();
    auto nextTask = getNextTask();
    ASSERT(nextTask);

    if(currentTask.get() != nextTask.get())
      Task::switchTo(rt::move(nextTask));
  }

  Result<result_t> kill(pid_t pid, status_t status)
  {
    {
      auto it = rt::find_if(activeTasksList().begin(), activeTasksList().end(), [pid](const rt::SharedPtr<Task>& task) { return task->pid == pid; });
      if(it == activeTasksList().end())
        return ErrorCode::INVALID;

      auto victim = *it;
      activeTasksList().erase(it);
      victim->kill(status);
      terminatedTasksList().insert(terminatedTasksList().end(), rt::move(victim));
    } // We need to ensure all local object is destructed before we call schedule

    schedule();
    return 0;
  }
}

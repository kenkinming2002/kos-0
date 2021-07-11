#include <generic/tasks/Scheduler.hpp>

#include <i686/interrupts/Interrupts.hpp>
#include <i686/tasks/Switch.hpp>

#include <x86/interrupts/8259.hpp>

#include <librt/Panic.hpp>
#include <librt/Global.hpp>
#include <librt/Iterator.hpp>
#include <librt/Log.hpp>
#include <librt/Assert.hpp>

#include <librt/containers/SharedList.hpp>

namespace core::tasks
{
  namespace
  {
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
    constinit rt::Global<rt::containers::SharedList<Task>> activeTasksList;
    constinit rt::Global<rt::containers::SharedList<Task>> terminatedTasksList;
  }

  namespace
  {
    void reaper()
    {
      for(;;)
      {
        if(!terminatedTasksList().empty())
        {
          auto task = terminatedTasksList().begin().get();
          terminatedTasksList().remove(terminatedTasksList().begin());
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
    return it.get();
  }

  namespace
  {
    rt::SharedPtr<Task> getNextTask()
    {
      if(activeTasksList().empty())
        return nullptr;

      auto nextTask = activeTasksList().begin().get();
      activeTasksList().splice(activeTasksList().end(), activeTasksList(), activeTasksList().begin(), rt::next(activeTasksList().begin()));
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
      auto it = rt::find_if(activeTasksList().begin(), activeTasksList().end(), [pid](const Task& task) { return task.pid == pid; });
      if(it == activeTasksList().end())
        return ErrorCode::INVALID;

      terminatedTasksList().splice(terminatedTasksList().end(), activeTasksList(), it, next(it));
      it->kill(status);
    } // We need to ensure all local object is destructed before we call schedule

    schedule();
    ASSERT(pid != Task::current()->pid); // If we kill ourself, we should not be able to reach here
    return 0;
  }
}

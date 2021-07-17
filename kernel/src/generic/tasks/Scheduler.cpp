#include <generic/tasks/Scheduler.hpp>

#include <i686/interrupts/Interrupts.hpp>
#include <i686/tasks/Switch.hpp>

#include <x86/interrupts/8259.hpp>

#include <librt/Panic.hpp>
#include <librt/Global.hpp>
#include <librt/Iterator.hpp>
#include <librt/Log.hpp>
#include <librt/Assert.hpp>

#include <librt/containers/List.hpp>

namespace core::tasks
{
  namespace
  {
    constinit rt::Global<rt::containers::List<rt::SharedPtr<Task>>> activeTasksList;
    constinit rt::Global<rt::containers::List<rt::SharedPtr<Task>>> terminatedTasksList;
  }

  namespace
  {
    /* Note: We have to disable interrupt, however, locking would not be
     *       necessary, since we would or *WILL* have a per-cpu task queue.  */
    void timerHandler(irq_t, uword_t, uintptr_t)
    {
      interrupts::acknowledge(0);
      schedule();
    }

    void initializeTimerInterrupt()
    {
      interrupts::installHandler(0x20, &timerHandler, PrivilegeLevel::RING0, true);
      interrupts::clearMask(0);
    }
  }

  namespace
  {
    // This should be a extremely low priority task
    void reaper()
    {
      for(;;)
      {
        if(!terminatedTasksList().empty())
        {
          auto task = *terminatedTasksList().begin();
          terminatedTasksList().remove(terminatedTasksList().begin());
          rt::logf("Reaping process pid = %ld, status = %ld\n", task->pid, task->status);
        }
        schedule();
      }
    }

    // This should be a task with lowest possibly imaginable priority
    void idle()
    {
      for(;;)
      {
        asm("sti");
        asm("hlt");
        asm("cli");
      }
    }

    void test()
    {
      for(size_t i=0; i<10; ++i)
      {
        asm("sti");
        asm("hlt");
        asm("cli");
      }
      return;
    }

    void initializeKernelTasks()
    {
      auto reaperTask = Task::allocate();
      reaperTask->asKernelTask(&reaper);
      addTask(rt::move(reaperTask));

      auto idleTask = Task::allocate();
      idleTask->asKernelTask(&idle);
      addTask(rt::move(idleTask));

      auto testTask = Task::allocate();
      testTask->asKernelTask(&test);
      addTask(rt::move(testTask));
    }
  }

  void initializeScheduler()
  {
    activeTasksList.construct();
    terminatedTasksList.construct();

    initializeTimerInterrupt();
    initializeKernelTasks();
  }

  void addTask(rt::SharedPtr<Task> task)
  {
    activeTasksList().insert(activeTasksList().end(), rt::move(task));
  }

  namespace
  {
    rt::SharedPtr<Task> getNextTask()
    {
      if(activeTasksList().empty())
        return nullptr;

      auto nextTask = *activeTasksList().begin();
      activeTasksList().splice(activeTasksList().end(), activeTasksList(), activeTasksList().begin(), rt::next(activeTasksList().begin()));
      return nextTask;
    }
  }

  void schedule()
  {
    auto currentTask = Task::current;
    auto nextTask = getNextTask();

    // We always have the reaper task
    ASSERT(nextTask);

    if(currentTask.get() != nextTask.get())
      Task::switchTo(rt::move(nextTask));
  }

  void killCurrent(status_t status)
  {
    kill(getpid(), status);
  }

  void onResume()
  {
    if(Task::current->state == Task::State::DEAD)
    {
      rt::log("rescheduling since current task is killed\n");
      schedule();
      __builtin_unreachable();
    }
  }

  pid_t getpid()
  {
    return Task::current->pid;
  }

  Result<result_t> kill(pid_t pid, status_t status)
  {
    auto it = rt::find_if(activeTasksList().begin(), activeTasksList().end(), [pid](const rt::SharedPtr<Task>& task) { return task->pid == pid; });
    if(it == activeTasksList().end())
      return ErrorCode::INVALID;

    terminatedTasksList().splice(terminatedTasksList().end(), activeTasksList(), it, next(it));
    (*it)->kill(status);

    /* We cannot reschedule at this moment since all objects from our calling
     * context may not yet be properly cleaned up */
    return 0;
  }

  Result<pid_t> fork()
  {
    auto clone = Task::current->clone();
    if(!clone)
      return ErrorCode::OUT_OF_MEMORY;

    clone->registers.eax = 0;
    addTask(clone);
    rt::logf("clone->pid:%ld\n", clone->pid);
    return clone->pid;
  }
}

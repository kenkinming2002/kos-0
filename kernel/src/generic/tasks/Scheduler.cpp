#include <generic/tasks/Scheduler.hpp>

#include <x86/interrupts/PIC.hpp>

#include <generic/Init.hpp>
#include <generic/PerCPU.hpp>
#include <generic/vfs/VFS.hpp>
#include <generic/tasks/Elf.hpp>

#include <librt/Panic.hpp>
#include <librt/Global.hpp>
#include <librt/Log.hpp>
#include <librt/Assert.hpp>
#include <librt/SpinLock.hpp>

#include <librt/containers/List.hpp>

namespace core::tasks
{
  namespace
  {
    struct Scheduler
    {
      std::atomic<unsigned> nextCpuid = 0;
      struct CpuInfo
      {
        rt::SpinLock activeTasksListLock;
        rt::SpinLock terminatedTasksListLock;

        rt::containers::List<rt::SharedPtr<Task>> activeTasksList;
        rt::containers::List<rt::SharedPtr<Task>> terminatedTasksList;
      };
      PerCPU<CpuInfo> cpuInfos;

      void reap()
      {
        auto& cpuInfo = cpuInfos.current();
        rt::LockGuard guard(cpuInfo.terminatedTasksListLock);
        if(!cpuInfo.terminatedTasksList.empty())
        {
          auto task = *cpuInfo.terminatedTasksList.begin();
          cpuInfo.terminatedTasksList.remove(cpuInfo.terminatedTasksList.begin());
          rt::logf("Reaping process pid = %ld, status = %ld\n", task->pid, task->status);
        }
      }

      void addTask(rt::SharedPtr<Task> task, unsigned cpuid)
      {
        rt::logf("Adding task to cpuid: %u\n", cpuid);

        auto& cpuInfo = cpuInfos.get(cpuid);
        rt::LockGuard guard(cpuInfo.activeTasksListLock);
        cpuInfo.activeTasksList.insert(cpuInfo.activeTasksList.end(), rt::move(task));
      }

      void addTask(rt::SharedPtr<Task> task)
      {
        unsigned cpuid = nextCpuid.fetch_add(1, std::memory_order_relaxed) % getCpusCount(); // There may be a bit of wrapping issue at the end but who cares?
        return addTask(rt::move(task), cpuid);
      }

      rt::SharedPtr<Task> getNextTask()
      {
        auto& cpuInfo = cpuInfos.current();
        rt::LockGuard guard(cpuInfo.activeTasksListLock);
        ASSERT(!cpuInfo.activeTasksList.empty());

        auto nextTask = *cpuInfo.activeTasksList.begin();
        cpuInfo.activeTasksList.splice(cpuInfo.activeTasksList.end(), cpuInfo.activeTasksList, cpuInfo.activeTasksList.begin(), rt::next(cpuInfo.activeTasksList.begin()));
        return nextTask;
      }

      Result<result_t> kill(pid_t pid, status_t status)
      {
        for(unsigned cpuid = 0; cpuid < getCpusCount(); ++cpuid)
        {
          auto& cpuInfo = cpuInfos.get(cpuid);
          rt::LockGuard guard1(cpuInfo.activeTasksListLock);

          auto it = rt::find_if(cpuInfo.activeTasksList.begin(), cpuInfo.activeTasksList.end(), [pid](const rt::SharedPtr<Task>& task) { return task->pid == pid; });
          if(it == cpuInfo.activeTasksList.end())
            continue;

          rt::LockGuard guard2(cpuInfo.terminatedTasksListLock);
          cpuInfo.terminatedTasksList.splice(cpuInfo.terminatedTasksList.end(), cpuInfo.activeTasksList, it, next(it));
          (*it)->kill(status);
          return 0;
        }
        return ErrorCode::INVALID;
      }
    };

    rt::Global<Scheduler> scheduler;
  }

  namespace
  {
    void timerHandler()
    {
      schedule();
    }

    void initializeTimer()
    {
      interrupts::addTimerCallback(&timerHandler);
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

    // This should be a extremely low priority task
    void reaper()
    {
      for(;;)
      {
        scheduler().reap();
        schedule();
      }
    }

    void initializeKernelTasks()
    {
      for(unsigned cpuid = 0; cpuid < getCpusCount(); ++cpuid)
      {
        auto idleTask = Task::allocate();
        idleTask->asKernelTask(&idle);
        addTask(rt::move(idleTask), cpuid);

        auto reaperTask = Task::allocate();
        reaperTask->asKernelTask(&reaper);
        addTask(rt::move(reaperTask), cpuid);
      }
    }

    void initializeInitTasks()
    {
      auto root = vfs::root();
      auto init = vfs::openAt(root, "init");
      if(!init)
        rt::panic("init not found\n");

      auto task = core::tasks::Task::allocate();
      if(!task)
        rt::panic("Failed to create task\n");

      core::tasks::loadElf(task, *init);
      core::tasks::addTask(task);
    }
  }

  void initializeScheduler()
  {
    scheduler.construct();
    initializeTimer();
    initializeKernelTasks();
    initializeInitTasks();
  }

  void addTask(rt::SharedPtr<Task> task, unsigned cpuid) { scheduler().addTask(rt::move(task), cpuid); }
  void addTask(rt::SharedPtr<Task> task)                 { scheduler().addTask(rt::move(task)); }

  void schedule()
  {
    auto currentTask = Task::current();
    auto nextTask = scheduler().getNextTask();

    interrupts::resetTimer();
    ASSERT(nextTask);
    if(currentTask.get() != nextTask.get())
    {
      ASSERT(nextTask->state == Task::State::RUNNABLE);
      Task::switchTo(rt::move(nextTask));
    }

  }

  void killCurrent(status_t status)
  {
    kill(getpid(), status);
  }

  void onResume()
  {
    if(Task::current()->state == Task::State::DEAD)
    {
      rt::log("rescheduling since current task is killed\n");
      schedule();
      __builtin_unreachable();
    }
  }

  pid_t getpid() { return Task::current()->pid; }
  Result<result_t> kill(pid_t pid, status_t status) { return scheduler().kill(pid, status); }

  Result<pid_t> fork()
  {
    auto clone = Task::current()->clone();
    if(!clone)
      return ErrorCode::OUT_OF_MEMORY;

    rt::logf("Forking\n");
    addTask(clone);
    return clone->pid;
  }
}

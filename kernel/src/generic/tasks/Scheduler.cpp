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
      rt::SpinLock                                    tasksMapLock;
      rt::containers::Map<pid_t, rt::SharedPtr<Task>> tasksMap;

      struct CpuInfo
      {
        rt::SpinLock activeTasksListLock;
        rt::containers::List<rt::SharedPtr<Task>> activeTasksList;

        rt::SpinLock terminatedTasksPidListLock;
        rt::containers::List<pid_t> terminatedTasksPidList;
      };
      PerCPU<CpuInfo> cpuInfos;

      PerCPU<rt::SharedPtr<Task>> blockedReaper;

    public:
      static void timerHandler(void* data)
      {
        auto& scheduler = *static_cast<Scheduler*>(data);
        scheduler.schedule();
      }

      static void idle(void* data)
      {
        auto& scheduler = *static_cast<Scheduler*>(data);
        for(;;)
        {
          asm("sti");
          asm("hlt");
          asm("cli");
        }
      }

      static void reaper(void* data)
      {
        auto& scheduler = *static_cast<Scheduler*>(data);
        for(;;)
        {
          scheduler.block(scheduler.blockedReaper.current());
          scheduler.reap();
          scheduler.schedule();
        }
      }

    public:
      Scheduler()
      {
        interrupts::addTimerCallback(&timerHandler, this);
        for(unsigned cpuid = 0; cpuid < getCpusCount(); ++cpuid)
        {
          auto idleTask = Task::allocate();
          idleTask->asKernelTask(&idle, this);
          addTask(rt::move(idleTask), cpuid);

          auto reaperTask = Task::allocate();
          reaperTask->asKernelTask(&reaper, this);
          addTask(rt::move(reaperTask), cpuid);
        }

        auto root = vfs::root();
        auto init = vfs::openAt(root, "init");
        if(!init)
          rt::panic("init not found\n");

        auto task = Task::allocate();
        if(!task)
          rt::panic("Failed to create task\n");

        core::tasks::loadElf(task, *init);
        addTask(task);
      }

    public:
      void reap()
      {
        auto& cpuInfo = cpuInfos.current();
        pid_t pid;
        {
          rt::LockGuard guard(cpuInfo.terminatedTasksPidListLock);
          if(cpuInfo.terminatedTasksPidList.empty())
            return;

          pid = *cpuInfo.terminatedTasksPidList.begin();
          cpuInfo.terminatedTasksPidList.remove(cpuInfo.terminatedTasksPidList.begin());
        }

        rt::LockGuard guard(tasksMapLock);
        auto it = tasksMap.find(pid);
        ASSERT(it != tasksMap.end());
        auto task = it->second;
        tasksMap.erase(it);

        rt::logf("Reaping process pid = %ld, status = %ld\n", task->schedInfo.pid, task->schedInfo.status);
      }

      void addTask(rt::SharedPtr<Task> task, unsigned cpuid, pid_t pid)
      {
        rt::logf("Adding task to cpuid: %u with pid %ld\n", cpuid, pid);
        task->schedInfo = {
          .cpuid = cpuid,
          .pid = pid,
          .state = SchedInfo::State::RUNNABLE,
          .pendingKill = false,
          .status = 0
        };
        {
          rt::LockGuard lock(tasksMapLock);
          tasksMap.insert({pid, task});
        }
        {
          auto& cpuInfo = cpuInfos.get(cpuid);
          rt::LockGuard guard(cpuInfo.activeTasksListLock);
          cpuInfo.activeTasksList.insert(cpuInfo.activeTasksList.end(), rt::move(task));
        }
      }

      void addTask(rt::SharedPtr<Task> task, unsigned cpuid)
      {
        static std::atomic<pid_t> nextPid = 0;
        auto pid = nextPid.fetch_add(1, std::memory_order_relaxed); // TODO: Detect wrapping
        addTask(rt::move(task), cpuid, pid);
      }

      void addTask(rt::SharedPtr<Task> task)
      {
        static constinit std::atomic<unsigned> nextCpuid = 0;
        auto cpuid = nextCpuid.fetch_add(1, std::memory_order_relaxed) % getCpusCount();
        addTask(rt::move(task), cpuid);
      }

      rt::SharedPtr<Task> getNextTask()
      {
        auto& cpuInfo = cpuInfos.current();
        rt::LockGuard guard(cpuInfo.activeTasksListLock);
        if(cpuInfo.activeTasksList.empty())
          return nullptr; // Nothing to schedule

        auto nextTask = *cpuInfo.activeTasksList.begin();
        cpuInfo.activeTasksList.remove(cpuInfo.activeTasksList.begin());
        return nextTask;
      }

      void pushPreviousTask(rt::SharedPtr<Task> previousTask)
      {
        auto& cpuInfo = cpuInfos.current();
        rt::LockGuard guard(cpuInfo.activeTasksListLock);
        cpuInfo.activeTasksList.insert(cpuInfo.activeTasksList.end(), rt::move(previousTask));
      }

      void schedule()
      {
        auto nextTask = getNextTask();
        if(!nextTask)
          return;

        {
          auto currentTask = Task::current();
          if(currentTask && currentTask->schedInfo.state != SchedInfo::State::BLOCKED)
          {
            currentTask->schedInfo.state = SchedInfo::State::RUNNABLE;
            pushPreviousTask(currentTask);
          }
        }

        nextTask->schedInfo.state = SchedInfo::State::RUNNING;
        interrupts::resetTimer();
        Task::switchTo(rt::move(nextTask));
      }

      void block(rt::SharedPtr<Task>& blockedTask)
      {
        {
          ASSERT(!blockedTask);
          blockedTask = Task::current();
          blockedTask->schedInfo.state = SchedInfo::State::BLOCKED;
          rt::logf("blocking task pid=0x%lx\n", blockedTask->schedInfo.pid);
        }

        rt::log("rescheduling since current task is blocked\n");
        schedule();
      }

      void unblock(rt::SharedPtr<Task> blockedTask)
      {
        ASSERT(blockedTask);
        rt::logf("unblocking task pid=0x%lx\n", blockedTask->schedInfo.pid);
        blockedTask->schedInfo.state = SchedInfo::State::RUNNABLE;

        auto& cpuInfo = cpuInfos.get(blockedTask->schedInfo.cpuid);
        rt::LockGuard guard(cpuInfo.activeTasksListLock);
        cpuInfo.activeTasksList.insert(cpuInfo.activeTasksList.end(), rt::move(blockedTask));
      }

      Result<result_t> kill(pid_t pid, status_t status)
      {
        rt::LockGuard lock(tasksMapLock);
        auto it = tasksMap.find(pid);
        if(it == tasksMap.end())
          return ErrorCode::INVALID; // Task not found

        rt::logf("Killing task pid=0x%lx\n", pid);
        it->second->schedInfo.pendingKill = true;
        it->second->schedInfo.status     = status;

        if(blockedReaper.current())
          unblock(rt::move(blockedReaper.current()));

        return 0;
      }

      void onResume()
      {
        if(Task::current()->schedInfo.pendingKill)
        {
          {
            auto& cpuInfo = cpuInfos.current();
            rt::LockGuard guard2(cpuInfo.terminatedTasksPidListLock);

            rt::logf("Inserting into terminated tasks pid list pid=%ld\n", Task::current()->schedInfo.pid);
            cpuInfo.terminatedTasksPidList.insert(cpuInfo.terminatedTasksPidList.end(), Task::current()->schedInfo.pid);
            ASSERT(!cpuInfo.terminatedTasksPidList.empty());

            rt::log("Removing current task\n");
            Task::current().reset();

            rt::log("rescheduling since current task is killed\n");
          }
          schedule();
          __builtin_unreachable();
        }

        ASSERT(Task::current()->schedInfo.state == SchedInfo::State::RUNNING);
      }
    };

    rt::Global<Scheduler> scheduler;
  }

  void initializeScheduler()
  {
    scheduler.construct();
  }

  void addTask(rt::SharedPtr<Task> task, unsigned cpuid) { scheduler().addTask(rt::move(task), cpuid); }
  void addTask(rt::SharedPtr<Task> task)                 { scheduler().addTask(rt::move(task)); }

  void schedule() { scheduler().schedule(); }

  void block(rt::SharedPtr<Task>& blockedTask)  { scheduler().block(blockedTask); }
  void unblock(rt::SharedPtr<Task> blockedTask) { scheduler().unblock(blockedTask); }

  void onResume() { scheduler().onResume(); }

  void killCurrent(status_t status) { kill(getpid(), status); }
  pid_t getpid() { return Task::current()->schedInfo.pid; }
  Result<result_t> kill(pid_t pid, status_t status) { return scheduler().kill(pid, status); }
  Result<pid_t> fork()
  {
    auto clone = Task::current()->clone();
    if(!clone)
      return ErrorCode::OUT_OF_MEMORY;

    rt::logf("Forking\n");
    addTask(clone);
    return clone->schedInfo.pid;
  }
}

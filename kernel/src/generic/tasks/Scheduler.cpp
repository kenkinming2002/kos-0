#include <generic/tasks/Scheduler.hpp>

#include <x86/interrupts/PIC.hpp>

#include <generic/Init.hpp>
#include <generic/PerCPU.hpp>
#include <generic/vfs/VFS.hpp>
#include <generic/tasks/Elf.hpp>

#include <generic/timers/Timer.hpp>

#include <generic/tasks/TasksMap.hpp>
#include <generic/tasks/RunQueue.hpp>
#include <generic/tasks/TerminatedQueue.hpp>

#include <librt/Panic.hpp>
#include <librt/Global.hpp>
#include <librt/Log.hpp>
#include <librt/Assert.hpp>
#include <librt/SpinLock.hpp>

#include <librt/containers/List.hpp>

namespace core::tasks
{
  /*
   * Design of Sccheduling System
   *
   * A Task could be in 4 states: RUNNING, RUNNABLE, BLOCKED, TERMIANTED
   *
   * The following invariant is maintained by taking the lock in Task SchedInfo
   *  - RUNNING:    On current
   *  - RUNNABLE:   On RunQueue
   *  - BLOCKED:    On WaitQueue
   *  - TERMINATED: On ReapQueue
   *
   *  With following transition:
   *  - Normal Scheduling Actions : RUNNING <-> RUNNABLE
   *  - Blocking and signaling    : RUNNING -> BLOCKED -> RUNNABLE
   *  -                             RUNNING -> TERMINATED
   */
  namespace
  {
    constinit TasksMap tasksMap;
    constinit PerCPU<RunQueue>        rqs;
    constinit PerCPU<TerminatedQueue> tqs;
    constinit PerCPU<WaitQueue>       wqsReaper;

    timers::Timer* timer;
  }

  void addTask(rt::SharedPtr<Task> task, unsigned cpuid)
  {
    static constinit std::atomic<unsigned> nextCpuid = 0;
    if(cpuid == UINT_MAX)
      cpuid = nextCpuid.fetch_add(1, std::memory_order_relaxed) % getCpusCount();

    task->schedInfo.cpuid = cpuid;
    tasksMap.addTask(task);

    task->schedInfo.state.store(SchedInfo::State::RUNNABLE);
    rqs.get(cpuid).enqueue(task);
  }

  /* RUNNING <=> RUNNABLE */
  rt::SharedPtr<Task> _schedule()
  {
    auto previousTask = Task::current();
    if(previousTask && previousTask->schedInfo.state.load() == SchedInfo::State::RUNNING)
    {
      previousTask->schedInfo.state.store(SchedInfo::State::RUNNABLE);
      rqs.current().enqueue(rt::move(previousTask));
    }

    auto nextTask = rqs.current().dequeue();
    ASSERT(nextTask->schedInfo.state.load() == SchedInfo::State::RUNNABLE);
    nextTask->schedInfo.state.store(SchedInfo::State::RUNNING);
    return rt::move(nextTask);
  }

  /* Run with preemption disabled */
  void schedule()
  {
    auto nextTask = _schedule();
    timer->reset();
    Task::switchTo(rt::move(nextTask));
  }

  WaitResult waitEvent(WaitQueue& wq, rt::FunctionRef<bool()> predicate)
  {
    WaitResult result;
    auto& currentTask = Task::current();

    auto it = wq.add(Task::current());
    currentTask->schedInfo.state.store(SchedInfo::State::BLOCKED);
    // Wakeup after this point would works
    for(;;)
    {
      if(predicate())
      {
        result = WaitResult::SUCCESS;
        break;
      }

      // TODO: Optimize for task e.g. kernel task with which all signals are blocked
      if(currentTask->sigInfo.pendingSignal())
      {
        result = WaitResult::INTERRUPTED;
        break;
      }

      schedule();
    }

    currentTask->schedInfo.state.store(SchedInfo::State::RUNNING);
    wq.remove(it);
    return result;
  }

  void wakeUp(rt::SharedPtr<Task>& task)
  {
    // TODO: Synchronize across multiple wake up
    if(task->schedInfo.state.load() == SchedInfo::State::BLOCKED)
    {
      if(task->schedInfo.state.exchange(SchedInfo::State::RUNNABLE) == SchedInfo::State::RUNNABLE)
        return; // Somebody got here ahead of us bail out

      rqs.get(task->schedInfo.cpuid).enqueue(task);
    }
  }

  void wakeUp(WaitQueue& wq)
  {
    auto task = wq.get();
    if(!task)
    {
      rt::log("Warning: no task in wait queue\n");
      return;
    }
    wakeUp(task);
  }

  void signal(rt::SharedPtr<Task>& task, Signal sig)
  {
    task->sigInfo.actions[static_cast<unsigned>(sig)].pending = true;
    wakeUp(task);
  }

  void handleSignal()
  {
    auto& currentTask = Task::current();
    for(auto& sigaction : currentTask->sigInfo.actions)
      if(sigaction.pending && sigaction.kill)
        currentTask->schedInfo.pendingKill = true;
  }

  void handlePendingKill()
  {
    auto& currentTask = Task::current();
    if(currentTask->schedInfo.pendingKill)
    {
      currentTask->schedInfo.state.store(SchedInfo::State::TERMINATED);
      tqs.current().enqueue(currentTask);
      wakeUp(wqsReaper.current());

      rt::log("Reschedule as current task is terminated\n");
      schedule();
      ASSERT_UNREACHABLE;
    }
  }

  void onResume()
  {
    handleSignal();
    handlePendingKill();
  }

  // System call
  Result<result_t> kill(pid_t pid, status_t status)
  {
    rt::logf("kill(pid=%ld, status=%ld)\n", pid, status);

    auto task = tasksMap.findTask(pid);
    if(!task)
      return ErrorCode::INVALID;


    task->schedInfo.status = status;
    signal(task, Signal::KILL);
    return 0;
  }

  pid_t getpid() { return Task::current()->schedInfo.pid; }
  Result<pid_t> fork()
  {
    auto clone = Task::current()->clone();
    if(!clone)
      return ErrorCode::OUT_OF_MEMORY;

    rt::logf("Forking\n");
    addTask(clone);
    return clone->schedInfo.pid;
  }

  /* Initialization and initial worker task */
  namespace
  {
    static void timerHandler(void* data)
    {
      schedule();
    }

    static void idle(void* data)
    {
      for(;;)
      {
        asm("sti");
        asm("hlt");
        asm("cli");
      }
    }

    static void reaper(void* data)
    {
      for(;;)
      {
        waitEvent(wqsReaper.current(), [](){ return !tqs.current().empty(); });
        tqs.current().reap(tasksMap);
      }
    }
  }

  void initializeScheduler()
  {
    // Preemption
    timer = timers::getSchedulerTimer();
    ASSERT(timer);
    timer->addCallback(&timerHandler, nullptr);

    // Idle and reaper task
    for(unsigned cpuid = 0; cpuid < getCpusCount(); ++cpuid)
    {
      auto idleTask = Task::allocate();
      idleTask->asKernelTask(&idle, nullptr);
      addTask(rt::move(idleTask), cpuid);

      auto reaperTask = Task::allocate();
      reaperTask->asKernelTask(&reaper, nullptr);
      addTask(rt::move(reaperTask), cpuid);
    }

    // Actual userspace init task
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

}

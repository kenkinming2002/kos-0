#include <generic/tasks/Scheduler.hpp>

#include <x86/interrupts/PIC.hpp>

#include <generic/Init.hpp>
#include <generic/PerCPU.hpp>
#include <generic/vfs/VFS.hpp>
#include <generic/tasks/Elf.hpp>

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
    interrupts::resetTimer();
    Task::switchTo(rt::move(nextTask));
  }

  /* prepareToBlock() and block() run synchronously
   * signal() run asynchronously
   *
   * prepareToBlock() must be ran before any call to block() and signal()
   * That mean before you run any "task" that may generate asynchrous signal,
   * you *must* call prepareToBlock() or you risk losing the signal.
   * */

  void prepareToBlock(WaitQueue& wq)
  {
    auto currentTask = Task::current();
    currentTask->schedInfo.state.store(SchedInfo::State::BLOCKED);
    wq.enqueue(currentTask);
  }

  void block()
  {
    auto currentTask = Task::current();
    while(!currentTask->schedInfo.siginfo.pendingSignal())
      schedule();
    currentTask->schedInfo.state.store(SchedInfo::State::RUNNING);
  }

  void signal(rt::SharedPtr<Task>& task, Signal sig)
  {
    task->schedInfo.siginfo.actions[static_cast<unsigned>(sig)].pending = true;
    if(task->schedInfo.state.load() == SchedInfo::State::BLOCKED)
    {
      task->schedInfo.state.store(SchedInfo::State::RUNNABLE);
      rqs.get(task->schedInfo.cpuid).enqueue(task);
    }
  }

  void signal(const WaitQueue& wq, Signal sig)
  {
    auto task = wq.get();
    if(!task)
    {
      rt::log("Warning: no task in wait queue\n");
      return;
    }

    signal(task, sig);
  }


  void signalOnce(WaitQueue& wq, Signal sig)
  {
    auto task = wq.dequeue(); // Sometimes, we do not want to dequeue the task
    if(!task)
    {
      rt::log("Warning: no task in wait queue\n");
      return;
    }

    signal(task, sig);
  }

  void handleSignal()
  {
    auto& currentTask = Task::current();
    for(auto& sigaction : currentTask->schedInfo.siginfo.actions)
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
    signal(task,                                 Signal::KILL);
    signal(wqsReaper.get(task->schedInfo.cpuid), Signal::IO);
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
      /* It is possible that we are woken up before we have a chance to call
       * prepareToBlock(). That does not matter, since this only happen in
       * system start up and we would clean up the next time we are woken up
       *
       * Ideally, we would be the first task scheduled and that *should* not be
       * a problem. */
      prepareToBlock(wqsReaper.current());
      for(;;)
      {
        block();
        auto signal = Task::current()->schedInfo.siginfo.get();
        ASSERT(signal == Signal::IO);
        tqs.current().reap(tasksMap);
      }
    }
  }

  void initializeScheduler()
  {
    // Preemption
    interrupts::addTimerCallback(&timerHandler, nullptr);

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

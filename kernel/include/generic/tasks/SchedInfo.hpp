#pragma once

#include <generic/tasks/Signals.hpp>

#include <librt/containers/Bitset.hpp>
#include <librt/SpinLock.hpp>

#include <sys/Types.hpp>

namespace core::tasks
{
  struct SchedInfo
  {
  public:
    /* Pid handling */
    pid_t pid;

  public:
    /* A task could be in the following state:
     *  RUNNING:  task is Task::current()
     *  RUNNABLE: task is on RunQueue
     *  TERMINATED: task is on ReapQueue
     *  BLOCKED: task is on waitlist
     *
     * lock is held whether state transition need to occur or state need to be
     * checked
     *
     * A task could always be found in a global tasksMap, which is
     * useful to look up task by pid, except when its state is TERMINATED.  */
    unsigned cpuid;
    enum class State
    {
      RUNNING,
      RUNNABLE,
      TERMINATED,
      BLOCKED,
      MIGRATING
    };
    std::atomic<State> state;

  public:
    /* Signal handling and return status */
    Siginfo siginfo;
    status_t status = 0;
    bool pendingKill = false;
  };
}

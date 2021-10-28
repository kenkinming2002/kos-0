#pragma once

#include <generic/tasks/WaitQueue.hpp>

#include <i686/tasks/Task.hpp>

#include <limits.h>

namespace core::tasks
{
  void initializeScheduler();

  void addTask(rt::SharedPtr<Task> task, unsigned cpuid = UINT_MAX);

  void schedule();

  void prepareToBlock(WaitQueue& wq);
  void block();

  void signal(rt::SharedPtr<Task>& task, Signal signal);
  void signal(const WaitQueue& wq, Signal sig);
  void signalOnce(WaitQueue& wq, Signal sig);

  /* Called when we are about to return to userspace, possibly from a
   * interrupt/syscall handler, to reschedule if the current task is killed for
   * any reason during the interrupt/syscall handler. It is a mistake to call
   * from any other context */
  void onResume();

  pid_t getpid();

  Result<result_t> kill(pid_t pid, status_t status);
  Result<pid_t> fork();

  inline void killCurrent(status_t status) { kill(getpid(), status); }
}

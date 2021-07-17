#pragma once

#include <i686/tasks/Task.hpp>

namespace core::tasks
{
  void initializeScheduler();

  void addTask(rt::SharedPtr<Task> task);

  void schedule();
  void killCurrent(status_t status);

  /* Called when we are about to return to userspace, possibly from a
   * interrupt/syscall handler, to reschedule if the current task is killed for
   * any reason during the interrupt/syscall handler. It is a mistake to call
   * from any other context */
  void onResume();

  pid_t getpid();
  Result<result_t> kill(pid_t pid, status_t status);
  Result<pid_t> fork();
}

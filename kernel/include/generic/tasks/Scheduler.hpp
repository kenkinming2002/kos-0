#pragma once

#include <i686/tasks/Task.hpp>

namespace core::tasks
{
  void initializeScheduler();

  void addTask(rt::SharedPtr<Task> task, unsigned cpuid);
  void addTask(rt::SharedPtr<Task> task);

  void schedule();

  void block(rt::SharedPtr<Task>& blockedTask);
  void unblock(rt::SharedPtr<Task> blockedTask);

  /* Called when we are about to return to userspace, possibly from a
   * interrupt/syscall handler, to reschedule if the current task is killed for
   * any reason during the interrupt/syscall handler. It is a mistake to call
   * from any other context */
  void onResume();

  void killCurrent(status_t status);
  pid_t getpid();
  Result<result_t> kill(pid_t pid, status_t status);
  Result<pid_t> fork();
}

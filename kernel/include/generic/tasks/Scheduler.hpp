#pragma once

#include <i686/tasks/Task.hpp>

namespace core::tasks
{
  void initializeScheduler();

  rt::SharedPtr<Task> addTask();
  void schedule();

  Result<result_t> kill(pid_t pid, status_t status);
}

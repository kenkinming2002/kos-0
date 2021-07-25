#include <generic/tasks/Tasks.hpp>

#include <generic/tasks/Syscalls.hpp>
#include <generic/tasks/Scheduler.hpp>

#include <i686/tasks/Task.hpp>

namespace core::tasks
{
  void initialize()
  {
    core::tasks::Task::intialize();
    core::tasks::initializeSyscalls();
    core::tasks::initializeScheduler();
  }
}

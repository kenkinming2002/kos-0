#include <generic/tasks/Tasks.hpp>

#include <generic/tasks/Syscalls.hpp>
#include <generic/tasks/Scheduler.hpp>

namespace core::tasks
{
  void initialize()
  {
    core::tasks::initializeSyscalls();
    core::tasks::initializeScheduler();
  }
}

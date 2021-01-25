#pragma once

#include <generic/utils/containers/List.hpp>
#include <i686/tasks/Task.hpp>

namespace core::tasks
{
  class Scheduler
  {
  public:
    Scheduler();

  public:
    [[noreturn]] void startFirstUserspaceTask();

  public:
    Task& addTask(Task task);
    void removeTask(Task& task);

  public:
    void schedule();
    void switchTask(Task& nextTask) const;

  private:
    utils::containers::List<Task> m_tasks;
  };

  extern Scheduler scheduler;
}

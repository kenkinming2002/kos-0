#pragma once

#include <generic/utils/containers/List.hpp>
#include <i686/tasks/Task.hpp>

namespace core::tasks
{
  class Scheduler
  {
  public:
    static Scheduler& instance()
    {
      static Scheduler scheduler;
      return scheduler;
    }

  public:
    Scheduler();

  public:
    [[noreturn]] void startFirstUserspaceTask();

  public:
    Task& addTask(Task task);
    void removeTask(Task& task);

  public:
    void schedule();

  private:
    utils::containers::List<Task> m_tasks;
  };
}

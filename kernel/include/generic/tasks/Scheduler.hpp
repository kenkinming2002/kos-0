#pragma once

#include <generic/utils/containers/List.hpp>
#include <i686/tasks/Task.hpp>

namespace core::tasks
{
  class Scheduler
  {
  public:
    static void initialize();
    static Scheduler& instance();

  public:
    Scheduler();

  public:
    [[noreturn]] void startFirstUserspaceTask();

  public:
    Task* addTask();
    Task& addTask(Task task);
    void removeTask(Task& task);

  public:
    void schedule();

  private:
    utils::containers::List<Task> m_tasks;
  };
}

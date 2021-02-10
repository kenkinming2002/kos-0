#pragma once

#include <i686/tasks/Task.hpp>

#include <librt/containers/List.hpp>

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
    rt::containers::List<Task> m_tasks;
  };
}

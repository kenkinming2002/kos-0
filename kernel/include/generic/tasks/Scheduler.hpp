#pragma once

#include "librt/UniquePtr.hpp"
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
    void removeTask(Task& task);

  public:
    void schedule();

  private:
    rt::containers::List<rt::UniquePtr<Task>> m_tasks;
  };
}

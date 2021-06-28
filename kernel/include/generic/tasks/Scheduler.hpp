#pragma once

#include "librt/SharedPtr.hpp"
#include <i686/tasks/Task.hpp>

#include <librt/UniquePtr.hpp>
#include <librt/containers/List.hpp>

namespace core::tasks
{
  void initializeScheduler();

  rt::SharedPtr<Task> addTask();
  void removeTask(Task* task);

  [[noreturn]] void scheduleInitial();
  void schedule();
}

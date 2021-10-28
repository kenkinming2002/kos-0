#pragma once

#include <generic/tasks/TasksMap.hpp>

#include <i686/tasks/Task.hpp>

#include <librt/Log.hpp>
#include <librt/SpinLock.hpp>
#include <librt/SharedPtr.hpp>
#include <librt/containers/List.hpp>

#include <limits.h>

namespace core::tasks
{
  struct RunQueue
  {
  private:
    rt::SpinLock lock;
    rt::containers::List<rt::SharedPtr<Task>> activeTasksList;

  public:
    void enqueue(rt::SharedPtr<Task> task)
    {
      rt::LockGuard guard(lock);
      activeTasksList.insert(activeTasksList.end(), rt::move(task));
    }

    rt::SharedPtr<Task> dequeue()
    {
      rt::LockGuard guard(lock);
      auto task = activeTasksList.front();
      activeTasksList.popFront();
      return task;
    }
  };

}

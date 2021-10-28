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
  struct WaitQueue
  {
  private:
    mutable rt::SpinLock lock;
    rt::containers::List<rt::SharedPtr<Task>> tasksList;

  public:
    void enqueue(rt::SharedPtr<Task> task)
    {
      rt::LockGuard guard1(lock);
      tasksList.insert(tasksList.end(), rt::move(task));
    }

  public:
    rt::SharedPtr<Task> dequeue()
    {
      rt::LockGuard guard(lock);
      if(tasksList.empty())
        return nullptr;

      auto task = tasksList.front();
      tasksList.popFront();
      return task;
    }

  public:
    rt::SharedPtr<Task> get() const
    {
      rt::LockGuard guard(lock);
      if(tasksList.empty())
        return nullptr;

      return tasksList.front();
    }
  };

}

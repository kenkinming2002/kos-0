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
    auto add(rt::SharedPtr<Task> task)
    {
      rt::LockGuard guard1(lock);
      return tasksList.insert(tasksList.end(), rt::move(task));
    }

    void remove(rt::containers::List<rt::SharedPtr<Task>>::const_iterator it)
    {
      rt::LockGuard guard1(lock);
      tasksList.remove(it);
    }

  public:
    rt::SharedPtr<Task> get()
    {
      rt::LockGuard guard(lock);
      if(tasksList.empty())
        return nullptr;

      return tasksList.front();
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

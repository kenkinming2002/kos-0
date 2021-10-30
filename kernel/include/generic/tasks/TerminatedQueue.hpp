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
  struct TerminatedQueue
  {
  public:
    mutable rt::SpinLock lock;
    rt::containers::List<rt::SharedPtr<Task>> terminatedTasksList;

  public:
    bool empty() const
    {
      rt::LockGuard guard(lock);
      return terminatedTasksList.empty();
    }


  public:
    void enqueue(rt::SharedPtr<Task> task)
    {
      rt::LockGuard guard1(lock);
      terminatedTasksList.insert(terminatedTasksList.end(), rt::move(task));
    }

  private:
    rt::SharedPtr<Task> dequeue()
    {
      rt::LockGuard guard(lock);
      if(terminatedTasksList.empty())
        return nullptr;

      auto task = terminatedTasksList.front();
      terminatedTasksList.popFront();
      return task;
    }

  public:
    void reap(TasksMap& tasksMap)
    {
      while(auto task = dequeue())
      {
        tasksMap.removeTask(task->schedInfo.pid);
        rt::logf("Terminated process pid = %ld, status = %ld\n", task->schedInfo.pid, task->schedInfo.status);
      }
    }
  };

}

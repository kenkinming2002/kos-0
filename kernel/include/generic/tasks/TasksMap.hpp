#pragma once

#include <i686/tasks/Task.hpp>

#include <librt/Log.hpp>

#include <librt/SpinLock.hpp>
#include <librt/SharedPtr.hpp>
#include <librt/containers/Map.hpp>

namespace core::tasks
{
  struct TasksMap
  {
    rt::SpinLock lock;
    std::atomic<pid_t> nextPid;
    rt::containers::Map<pid_t, rt::SharedPtr<Task>> map;

    void addTask(rt::SharedPtr<Task> task)
    {
      auto pid = nextPid.fetch_add(1, std::memory_order_relaxed);
      task->schedInfo.pid = pid;

      rt::LockGuard guard1(lock);
      map.insert({pid, rt::move(task)});
    }

    rt::SharedPtr<Task> findTask(pid_t pid)
    {
      rt::LockGuard guard(lock);
      auto it = map.find(pid);
      return it != map.end() ? it->second : nullptr;
    }

    void removeTask(pid_t pid)
    {
      rt::LockGuard guard(lock);
      auto it = map.find(pid);
      if(it == map.end())
      {
        rt::logf("Warning: Attempting to remove non-existent task with pid = %lx\n", pid);
        return;
      }
      map.erase(it);
    }
  };
}

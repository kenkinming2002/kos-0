#include <generic/tasks/Scheduler.hpp>

#include <i686/syscalls/Syscalls.hpp>
#include <i686/interrupts/Interrupts.hpp>
#include <i686/tasks/Switch.hpp>

#include <x86/interrupts/8259.hpp>

#include <librt/Panic.hpp>
#include <librt/Global.hpp>
#include <librt/Iterator.hpp>
#include <librt/Log.hpp>
#include <librt/Assert.hpp>

namespace core::tasks
{
  namespace
  {
    constinit rt::Global<rt::containers::List<rt::UniquePtr<Task>>> tasks;

    int syscallYield(int, int, int, int, int, int, int)
    {
      schedule();
      return 0;
    }

    /* Note: We have to disable interrupt, however, locking would not be
     *       necessary, since we would or *WILL* have a per-cpu task queue.  */
    void timerHandler(uint8_t, uint32_t, uintptr_t)
    {
      interrupts::acknowledge(0);
      schedule();
    }

    auto findTask(Task* task)
    {
      ASSERT(task);

      // FIXME: We can do better than that
      for(auto it = tasks().begin(); it!=tasks().end(); ++it)
        if(it->get() == task)
          return it;

      rt::panic("Failed to find task\n");
    }
  }

  void initializeScheduler()
  {
    tasks.construct();
    syscalls::installHandler(0, &syscallYield);
    interrupts::installHandler(0x20, &timerHandler, PrivilegeLevel::RING0, true);
    interrupts::clearMask(0);
  }

  Task* addTask()
  {
    auto task = Task::allocate();
    if(!task)
      return nullptr;

    auto it = tasks().insert(tasks().end(), rt::move(task));
    return it->get();
  }

  void removeTask(Task* task)
  {
    auto it = findTask(task);
    tasks().erase(it);
  }

  [[noreturn]] void scheduleInitial()
  {
    rt::log("We are starting our first task\n");
    tasks().begin()->get()->switchTo();
    __builtin_unreachable();
  }

  void schedule()
  {
    ASSERT(!tasks().empty());
    auto currentTask = Task::current();
    auto it = findTask(currentTask);
    auto nextIt = rt::next(it) != tasks().end() ? rt::next(it) : tasks().begin();
    (*nextIt)->switchTo();
  }
}

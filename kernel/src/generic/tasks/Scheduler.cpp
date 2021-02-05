#include <generic/tasks/Scheduler.hpp>

#include <common/generic/io/Print.hpp>
#include <i686/syscalls/Syscalls.hpp>
#include <i686/interrupts/Interrupts.hpp>
#include <x86/interrupts/8259.hpp>
#include <i686/tasks/Switch.hpp>

#include <functional>
#include <assert.h>

namespace core::tasks
{
  /* 
   * Yield syscall 
   *
   * Note: We can just use cli and sti instruction instead of
   *       saving and restoring eflags. This is based on the assumption that
   *       syscall must come from userspace and thus, interrupt must be enabled.
   */
  static int syscallYield(int, int, int, int)
  { 
    asm volatile ("cli" : : : "memory");
    Scheduler::instance().schedule(); 
    asm volatile ("sti" : : : "memory");
    return 0;
  }

  /* 
   * Note: We have to disable interrupt, however, locking would not be
   *       necessary, since we would or *WILL* have a per-cpu task queue. 
   */
  static void timerHandler(uint8_t, uint32_t, uintptr_t)
  { 
    interrupts::acknowledge(0); 
    Scheduler::instance().schedule(); 
  }

  Scheduler::Scheduler()
  {
    syscalls::installHandler(0, &syscallYield);
    interrupts::installHandler(0x20, &timerHandler, PrivilegeLevel::RING0, true);
    interrupts::clearMask(0);
  }

  [[noreturn]] void Scheduler::startFirstUserspaceTask()
  {
    io::print("We are starting our first task\n");

    syscalls::initialize();
    m_tasks.begin()->switchTo();
    __builtin_unreachable();
  }

  Task* Scheduler::addTask()
  {
    auto task = Task::allocate();
    if(!task)
      return nullptr;

    return &addTask(std::move(*task));
  }

  Task& Scheduler::addTask(Task task)
  {
    auto it = m_tasks.insert(m_tasks.end(), std::move(task));
    return *it;
  }

  void Scheduler::removeTask(Task& task)
  {
    // FIXME: We can do better than that
    for(auto it = m_tasks.begin(); it!=m_tasks.end(); ++it)
      if(&(*it) == &task)
      {
        m_tasks.erase(it);
        return;
      }
  }

  void Scheduler::schedule()
  {
    assert(!m_tasks.empty());

    // We can actually convert pointer to iterator directly if we export 
    // a interface from the container to do so
    for(auto it = m_tasks.begin(); it != std::prev(m_tasks.end()); ++it)
      if(Task::current == &(*it))
      {
        std::next(it)->switchTo();
        return;
      }

    m_tasks.begin()->switchTo();
  }
}

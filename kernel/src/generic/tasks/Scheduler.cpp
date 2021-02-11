#include <generic/tasks/Scheduler.hpp>

#include <i686/syscalls/Syscalls.hpp>
#include <i686/interrupts/Interrupts.hpp>
#include <i686/tasks/Switch.hpp>

#include <x86/interrupts/8259.hpp>

#include <librt/Global.hpp>
#include <librt/Iterator.hpp>
#include <librt/Log.hpp>
#include <librt/Assert.hpp>

namespace core::tasks
{
  static rt::Global<Scheduler> scheduler;
  void Scheduler::initialize()
  {
    scheduler.construct();
  }

  Scheduler& Scheduler::instance()
  {
    return scheduler();
  }

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
    rt::log("We are starting our first task\n");

    syscalls::initialize();
    m_tasks.begin()->get()->switchTo();
    __builtin_unreachable();
  }

  Task* Scheduler::addTask()
  {
    auto task = Task::allocate();
    if(!task)
      return nullptr;

    auto it = m_tasks.insert(m_tasks.end(), rt::move(task));
    return it->get();
  }

  void Scheduler::removeTask(Task& task)
  {
    // FIXME: We can do better than that
    for(auto it = m_tasks.begin(); it!=m_tasks.end(); ++it)
      if(it->get() == &task)
      {
        m_tasks.erase(it);
        return;
      }
  }

  void Scheduler::schedule()
  {
    ASSERT(!m_tasks.empty());

    // We can actually convert pointer to iterator directly if we export 
    // a interface from the container to do so
    for(auto it = m_tasks.begin(); it != rt::prev(m_tasks.end()); ++it)
      if(Task::current() == it->get())
      {
        rt::next(it)->get()->switchTo();
        return;
      }

    m_tasks.begin()->get()->switchTo();
  }
}

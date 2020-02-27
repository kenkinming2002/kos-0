#include <i686/core/multiprocessing/Yield.hpp>

#include <i686/core/MultiProcessing.hpp>

#include <i686/core/Syscall.hpp>

namespace core::multiprocessing
{
  namespace
  {
    int syscall_yield(const State state)
    {
      // Save current process state
      auto& currentProcess = processesList.front();
      currentProcess.state(state);

      // set next process as current
      processesList.shift_forward(1);

      auto& nextProcess = processesList.front();

      // NOTE: This cause unnecessary flushing of the TLB if no actual task
      //       switch is done(i.e. switching to the same task)
      nextProcess.setAsActive(); 
      nextProcess.run();

      return 0;
    }
  }

  void init()
  {
    core::register_syscall_handler(0x4, &syscall_yield);
  }
}

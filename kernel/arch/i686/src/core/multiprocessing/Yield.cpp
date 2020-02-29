#include <i686/core/multiprocessing/Yield.hpp>

#include <i686/core/MultiProcessing.hpp>

#include <i686/core/Syscall.hpp>
#include <generic/io/Print.hpp>

#include <i686/core/multiprocessing/SwitchProcess.hpp>

namespace core::multiprocessing
{
  namespace
  {
    extern "C" void schedule()
    {
      processesList.shift_forward(1);
    }

    int syscall_yield(const State /*state*/)
    {
      auto& previousProcess = processesList.front();
      schedule();
      auto& nextProcess = processesList.front();
      nextProcess.setStackAsActive();

      switch_process(&previousProcess, &nextProcess);
      return 0;
    }
  }

  void init()
  {
    core::register_syscall_handler(0x4, &syscall_yield);
  }
}

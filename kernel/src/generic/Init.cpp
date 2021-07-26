#include <generic/Init.hpp>

#include <generic/BootInformation.hpp>

#include <atomic>

namespace core
{
  inline void pause() { asm volatile("pause"); }

  class ConditionalVariable
  {
  public:
    void notify(unsigned total)
    {
      count = 0; // (1)
      flag = true; // (2)
      while(count.load() != total) pause(); // (5)
      flag = false; // (6)
    }

    void wait()
    {
      while(!flag.load()) pause(); // (3)
      ++count; // (4)
      while(flag.load()) pause(); // (7)
    }

  private:
    std::atomic<bool> flag = false;
    std::atomic<unsigned> count = 0;
  };

  namespace
  {
    constinit std::atomic<init_func_t> currentFunc;
    constinit ConditionalVariable cond_newFunction;
    constinit ConditionalVariable cond_completion;
  }

  /* 1) Signal other CPU that a new function is available
   * 2) Wait for all function call to complete */

  void foreachCPUInitCall(init_func_t func)
  {
    currentFunc = func;
    cond_newFunction.notify(bootInformation->coresCount-1);
    func();
    cond_completion.notify(bootInformation->coresCount-1);
  }

  void foreachCPUInitHandleOnce()
  {
    cond_newFunction.wait();
    auto func = currentFunc.load();
    func();
    cond_completion.wait();
  }

  void foreachCPUInitHandleLoop()
  {
    for(;;)
      foreachCPUInitHandleOnce();
  }
}

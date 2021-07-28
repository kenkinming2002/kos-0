#include <generic/Init.hpp>

#include <generic/BootInformation.hpp>

#include <librt/ConditionalVariable.hpp>

#include <atomic>

namespace core
{
  namespace
  {
    constinit init_func_t currentFunc;
    constinit rt::ConditionalVariable cond_newFunction;
    constinit rt::ConditionalVariable cond_completion;
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
    currentFunc();
    cond_completion.wait();
  }

  void foreachCPUInitHandleLoop()
  {
    for(;;)
      foreachCPUInitHandleOnce();
  }
}

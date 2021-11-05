#include <x86/SMP.hpp>

#include <generic/Init.hpp>

#include <atomic>

namespace
{
  std::atomic<bool> bsp;
  std::atomic<unsigned> initializedCoresCount;
}

namespace core
{

  [[noreturn]] static void runAP()
  {
    core::foreachCPUInitHandleLoop();
  }

  // Only return for BSP
  void initializeSMP(BootInformation* bootInformation, unsigned apicid)
  {
    initializedCoresCount.fetch_add(1, std::memory_order_release);
    while(initializedCoresCount.load(std::memory_order_acquire) != bootInformation->coresCount)
      asm volatile("pause");

    if(bsp.exchange(true, std::memory_order_relaxed))
      runAP();
  }
}

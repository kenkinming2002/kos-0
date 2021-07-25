#include <generic/PerCPU.hpp>

#include <generic/Init.hpp>
#include <generic/BootInformation.hpp>

#include <librt/Log.hpp>

#include <atomic>
#include <cpuid.h>

namespace core
{
  static constexpr auto MAX_APICID = 16;

  namespace
  {
    std::atomic<unsigned> cpuidLast;
    unsigned cpuids[MAX_APICID];
    unsigned apicidCurrent()
    {
      int ebx, discard;
      __cpuid(0x1, discard, ebx, discard, discard);
      return ebx >> 0x18;
    }

    unsigned cpusCount;
  }

  void initializePerCPU()
  {
    foreachCPUInitCall([](){
      auto apicid = apicidCurrent();
      cpuids[apicid] = cpuidLast++;
      rt::logf("Assigned apicid %u -> cpuid %u\n", apicid, cpuids[apicid]);
    });

    cpusCount = bootInformation->coresCount;
  }

  unsigned cpuidCurrent()
  {
    auto apicid = apicidCurrent();
    return cpuids[apicid];
  }

  unsigned getCpusCount()
  {
    return cpusCount;
  }
}

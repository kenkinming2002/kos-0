#include <generic/Modules.hpp>

#include <generic/BootInformation.hpp>
#include <generic/Init.hpp>
#include <common/generic/io/Print.hpp>

namespace core
{
  using namespace common;

  size_t modulesCount;
  Module* modules;

  INIT_FUNCTION_EARLY void initModules()
  {
    modulesCount = bootInformation->moduleEntriesCount ;
    modules = new Module[modulesCount];
    for(size_t i=0; i<bootInformation->moduleEntriesCount; ++i)
    {
      core::io::printf("Module phyaddr:%lx, length:%lx\n", bootInformation->moduleEntries[i].addr, bootInformation->moduleEntries[i].len);
      modules[i] = Module{bootInformation->moduleEntries[i].addr, bootInformation->moduleEntries[i].len};
    }
  }
}

#include <core/generic/Modules.hpp>

#include <boot/i686/BootInformation.hpp>
#include <core/generic/Init.hpp>
#include <core/generic/io/Print.hpp>

namespace core
{
  size_t modulesCount;
  Module* modules;

  INIT_FUNCTION_EARLY void initModules()
  {
    modulesCount = BOOT_INFORMATION.moduleEntriesCount ;
    modules = new Module[modulesCount];
    for(size_t i=0; i<BOOT_INFORMATION.moduleEntriesCount; ++i)
    {
      core::io::printf("Module phyaddr:%lx, length:%lx\n", BOOT_INFORMATION.moduleEntries[i].addr, BOOT_INFORMATION.moduleEntries[i].len);
      modules[i] = Module{BOOT_INFORMATION.moduleEntries[i].addr, BOOT_INFORMATION.moduleEntries[i].len};
    }
  }
}

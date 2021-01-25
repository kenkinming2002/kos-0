#include <generic/icxxabi.hpp>

#include <generic/BootInformation.hpp>

extern "C" void _init();
extern "C" void _fini();
void kmain();

extern "C" void _start(BootInformation* bootInformation)
{
  ::bootInformation = bootInformation;

  _init();
  kmain();
  __cxa_finalize(nullptr);
  _fini();
}


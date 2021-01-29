#include <generic/icxxabi.hpp>

#include <generic/BootInformation.hpp>
#include <generic/memory/Memory.hpp>

#include <i686/internals/Internals.hpp>
#include <i686/interrupts/Interrupts.hpp>

extern "C" void _init();
extern "C" void _fini();
void kmain();

extern "C" void _start(BootInformation* bootInformation)
{
  ::bootInformation = bootInformation;

  core::internals::initialize();
  core::interrupts::initialize();

  _init();
  kmain();
  __cxa_finalize(nullptr);
  _fini();
}


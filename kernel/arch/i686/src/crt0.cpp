#include <generic/icxxabi.hpp>

#include <i686/core/Interrupt.hpp>
#include <intel/core/pic/8259.hpp>
#include <intel/core/Exceptions.hpp>

extern "C" void _init();
extern "C" void _fini();

extern "C" void kmain();

extern "C" void _start()
{
  core::interrupt::init();
  core::pic::controller8259::init();
  core::exceptions::init();

  _init();
  kmain();
  __cxa_finalize(nullptr);
  _fini();
}

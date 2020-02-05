#include <generic/icxxabi.hpp>

extern "C" void _init();
extern "C" void _fini();

extern "C" void kmain();

extern "C" void _start()
{
  _init();
  kmain();
  __cxa_finalize(nullptr);
  _fini();
}

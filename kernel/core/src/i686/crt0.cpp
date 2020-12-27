#include <core/generic/icxxabi.hpp>

extern "C" void _init();
extern "C" void _fini();
void kmain();

extern "C" void _start()
{
  _init();
  kmain();
  __cxa_finalize(nullptr);
  _fini();
}


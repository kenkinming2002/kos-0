#include <librt/Panic.hpp>

extern "C" void __cxa_pure_virtual()
{
  rt::panic("Pure virtual function called");
}

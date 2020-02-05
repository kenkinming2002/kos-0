#include <i686/boot/higher_half.hpp>

extern "C" void _init();

extern "C" void higher_half_main()
{
  _init();
}

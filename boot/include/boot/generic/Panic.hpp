#include <common/generic/io/Print.hpp>

[[noreturn]] inline void panic(const char* msg)
{
  common::io::print(msg);
  for(;;) asm volatile("hlt");
}

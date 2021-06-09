#include "Syscalls.hpp"

#include <stddef.h>

extern "C" void main()
{
#if 0
  for(size_t i=0; i<10; ++i)
  {
    const char* msg = "bye world\n";
    syscall(13, i, 0, 0, 0, 0, 0);
    syscall(14, reinterpret_cast<uword_t>(msg), 10, 0, 0, 0, 0);
  }
#endif
}

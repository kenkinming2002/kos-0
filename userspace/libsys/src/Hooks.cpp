#include <libsys/Syscalls.hpp>

namespace rt::hooks
{
  void log(const char* str, size_t length) { sys_log(str, length); }
  [[noreturn]] void abort() { for(;;); }

}


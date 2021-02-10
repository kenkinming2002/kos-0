#include <librt/Log.hpp>

extern "C" void __assert_func(const char *file, int line, const char *func, const char *failedexpr)
{
  rt::logf("Assertion Failure:\n");
  rt::logf("  line: %d\n", line);
  rt::logf("  function: %s\n", func);
  rt::logf("  failedexpr: %s\n", failedexpr);
  for(;;)
    asm("hlt");
}

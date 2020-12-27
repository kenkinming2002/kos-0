#include <core/generic/io/Print.hpp>

#ifdef __cplusplus
extern "C" {
#endif

void __assert_func(const char *file, int line, const char *func, const char *failedexpr)
{
  core::io::printf("Assertion Failure:\n");
  core::io::printf("  line: %d\n", line);
  core::io::printf("  function: %s\n", func);
  core::io::printf("  failedexpr: %s\n", failedexpr);
  for(;;)
    asm("hlt");
}

#ifdef __cplusplus
}
#endif

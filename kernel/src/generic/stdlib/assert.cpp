#include <common/generic/io/Print.hpp>

#ifdef __cplusplus
extern "C" {
#endif

void __assert_func(const char *file, int line, const char *func, const char *failedexpr)
{
  common::io::printf("Assertion Failure:\n");
  common::io::printf("  line: %d\n", line);
  common::io::printf("  function: %s\n", func);
  common::io::printf("  failedexpr: %s\n", failedexpr);
  for(;;)
    asm("hlt");
}

#ifdef __cplusplus
}
#endif

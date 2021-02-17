#include <librt/Log.hpp>
#include <librt/Hooks.hpp>

namespace rt::internals
{
  void assertFunc(const char* file, int line, const char* func, const char* expr)
  {
    rt::logf("Assertion Failure:\n");
    rt::logf("  line: %d\n", line);
    rt::logf("  function: %s\n", func);
    rt::logf("  expr: %s\n", expr);
    rt::hooks::abort();
  }
}

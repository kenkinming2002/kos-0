#include <librt/Log.hpp>

#include <librt/Hooks.hpp>
#include <librt/Trace.hpp>

namespace rt::internals
{
  [[noreturn]] void assertFunc(const char* file, int line, const char* func, const char* expr)
  {
    logf("Assertion Failure:\n");
    logf("  line: %d\n", line);
    logf("  function: %s\n", func);
    logf("  expr: %s\n", expr);
    traceStackFrame();
    hooks::abort();
  }
}

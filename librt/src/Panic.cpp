#include <librt/Panic.hpp>

#include <librt/Log.hpp>
#include <librt/Hooks.hpp>
#include <librt/Trace.hpp>

namespace rt
{
  [[noreturn]] void panic(const char* format, ...)
  {
    log("Panic:\n  ");

    va_list ap;
    va_start(ap, format);
    vlogf(format, ap);
    va_end(ap);

    traceStackFrame();

    hooks::abort();
  }
}

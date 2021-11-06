#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

namespace rt
{
  // TODO: I really dislike logf but implementing any alternative would
  //       requires substantial effort, so that is what we have for now.
  void vlogf(const char* format, va_list ap);
  [[gnu::format(printf,1,2)]] void logf(const char* format, ...);
}

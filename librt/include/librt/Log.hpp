#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

namespace rt
{
  void log(const char* str, size_t length);
  void log(const char* str);
  void log(unsigned value, unsigned base);
  void log(long unsigned value, unsigned base);
  void log(int value, unsigned base);
  void log(long int value, unsigned base);

  // TODO: I really dislike logf but implementing any alternative would
  //       requires substantial effort, so that is what we have for now.
  [[gnu::format(printf,1,2)]] void logf(const char* format, ...);
  void vlogf(const char* format, va_list ap);
}

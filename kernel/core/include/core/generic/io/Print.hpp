#pragma once

#include <string_view>

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

namespace core::io
{
  void print(std::string_view str);

  void print(unsigned value, unsigned base);
  void print(long unsigned value, unsigned base);

  void print(int value, unsigned base);
  void print(long int value, unsigned base);

  // TODO: I really dislike printf but implementing any alternative would
  //       requires substantial effort, so that is what we have for now.
  __attribute__((format(printf,1,2))) void printf(const char* format, ...);
  void vprintf(const char* format, va_list ap);
}

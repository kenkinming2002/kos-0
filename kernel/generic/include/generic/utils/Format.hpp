#pragma once

#include <stdint.h>
#include <stddef.h>
#include <type_traits>

namespace utils
{
  using string_type = const char*;

  template<typename T, typename = void>
  struct format_string_length;

  template<typename T>
  struct format_string_length<T, std::enable_if_t<std::is_unsigned_v<T>>>
    : std::integral_constant<size_t, 2 * sizeof(T)/* Each byte can be represented by 2 characters in hex */> {};

  string_type format(uint16_t i);
  string_type format(uint32_t i);
  string_type format(uint64_t i);

  int format(char* buf, size_t len, uint16_t i);
  int format(char* buf, size_t len, uint32_t i);
  int format(char* buf, size_t len, uint64_t i);

  int format(char* buf, size_t len, const char* str);
}

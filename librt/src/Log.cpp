#include <librt/Log.hpp>

#include <librt/Strings.hpp>
#include <librt/Hooks.hpp>
#include <librt/Algorithm.hpp>

#include <type_traits>

#include <limits.h>

namespace rt
{
  static void snlog(char*& buf, size_t& size, const char* str, size_t length)
  {
    auto n = rt::min(size, length);
    rt::copy_n(str, buf, n);
    buf  += n;
    size -= n;

  }

  static void snlog(char*& buf, size_t& size, const char* str)
  {
    snlog(buf, size, str, strlen(str));
  }

  template<typename T> requires std::is_integral_v<T> && std::is_unsigned_v<T>
  static void snlog(char*& buf, size_t& size, T value, unsigned base)
  {
    char _buf[sizeof value * CHAR_BIT];
    char* end = &_buf[sizeof _buf - 1];
    char* ptr = end - 1;
    for(; value != 0; value /= base)
    {
      auto digit = value % base;
      *ptr-- = digit<10 ? '0' + digit : digit<16 ? 'A' + digit : 'X';
    }
    if(ptr ==  end - 1)
      *ptr-- = '0';

    snlog(buf, size, ptr+1, end - (ptr+1));
  }

  template<typename T> requires std::is_integral_v<T> && std::is_signed_v<T>
  static void snlog(char*& buf, size_t& size, T value, unsigned base)
  {
    if(value<0)
    {
      snlog(buf, size, "-");
      value = -value;
    }

    snlog(buf, size, static_cast<unsigned>(value), base);
  }

  static void vsnlogf(char*& buf, size_t& size, const char* format, va_list ap)
  {
    const char *begin = format, *it = format;
    auto flush = [&](){ snlog(buf, size, begin, it-begin); };

    while(*it != 0)
    {
      if(*it=='%')
      {
        flush();
        switch(it[1])
        {
        case 's':
          snlog(buf, size, va_arg(ap, const char*));
          break;

        // Decimal Integer
        case 'u':
          snlog(buf, size, va_arg(ap, unsigned), 10);
          break;
        case 'd':
        case 'i':
          snlog(buf, size, va_arg(ap, int), 10);
          break;

        // Hexadecimal Integer
        case 'x': // TODO: Implement lower case haxadecimal
        case 'X':
          snlog(buf, size, va_arg(ap, unsigned), 16);
          break;

        // Long
        case 'l':
          switch(it[2])
          {
          case 'u':
            snlog(buf, size, va_arg(ap, long unsigned), 10);
            break;
          case 'd':
          case 'i':
            snlog(buf, size, va_arg(ap, long int), 10);
            break;

          // Hexadecimal Integer
          case 'x': // TODO: Implement lower case haxadecimal
          case 'X':
            snlog(buf, size, va_arg(ap, long unsigned), 16);
            break;
          }
          ++it;
          break;

        case '\0':
          return; // WTH
        }
        ++it;
        begin = it+1;
      }

      ++it;
    }
    flush();
  }

  void vlogf(const char* format, va_list ap)
  {
    // This could be a problem if it is too large
    char _buf[128];
    char* buf   = _buf;
    size_t size = sizeof _buf;
    vsnlogf(buf, size, format, ap);
    hooks::log(_buf, buf - _buf);
  }

  void logf(const char* format, ...)
  {
    va_list ap;
    va_start(ap, format);
    vlogf(format, ap);
    va_end(ap);
  }

}

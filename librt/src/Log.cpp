#include <librt/Log.hpp>

#include <librt/Strings.hpp>
#include <librt/Hooks.hpp>
#include <librt/SpinLock.hpp>

#include <limits.h>

namespace rt
{
  void log(const char* str, size_t length)
  {
    static SpinLock lock;
    LockGuard guard(lock);

    hooks::log(str, length);
  }

  void log(const char* str)
  {
    static SpinLock lock;
    LockGuard guard(lock);

    log(str, strlen(str));
  }

  void log(unsigned value, unsigned base)
  {
    static SpinLock lock;
    LockGuard guard(lock);

    if(base==0 || base>16)
      return log("NaN");

    char buf[sizeof value * CHAR_BIT+1];
    int index = sizeof buf;

    for(; value!=0; value/= base)
    {
      auto digit = value % base;
      if(digit>=0 && digit<10)
        buf[--index] = '0' + digit;
      else if(digit>=10 && digit<16)
        buf[--index] = 'A'+(digit-10);
    }

    if(index != sizeof buf)
      log(&buf[index], sizeof buf - index);
    else
      log("0");
  }

  void log(long unsigned value, unsigned base)
  {
    static SpinLock lock;
    LockGuard guard(lock);

    if(base==0 || base>16)
      return log("NaN");

    char buf[sizeof value * CHAR_BIT+1];
    int index = sizeof buf;

    for(; value!=0; value/= base)
    {
      auto digit = value % base;
      if(digit>=0 && digit<10)
        buf[--index] = '0' + digit;
      else if(digit>=10 && digit<16)
        buf[--index] = 'A'+(digit-10);
    }

    if(index != sizeof buf)
      log(&buf[index], sizeof buf - index);
    else
      log("0");
  }

  void log(int value, unsigned base)
  {
    static SpinLock lock;
    LockGuard guard(lock);

    if(value>=0)
    {
      log(static_cast<unsigned>(value), base);
    }
    else
    {
      log("-");
      log(static_cast<unsigned>(-value), base);
    }
  }

  void log(long int value, unsigned base)
  {
    static SpinLock lock;
    LockGuard guard(lock);

    if(value>=0)
    {
      log(static_cast<long unsigned>(value), base);
    }
    else
    {
      log("-");
      log(static_cast<long unsigned>(-value), base);
    }
  }

  void logf(const char* format, ...)
  {
    static SpinLock lock;
    LockGuard guard(lock);

    va_list ap;
    va_start(ap, format);
    vlogf(format, ap);
    va_end(ap);
  }

  void vlogf(const char* format, va_list ap)
  {
    static SpinLock lock;
    LockGuard guard(lock);

    const char *begin = format, *it = format;
    auto flush = [&](){ log(begin, it-begin); };

    while(*it != 0)
    {
      if(*it=='%')
      {
        flush();
        switch(it[1])
        {
        case 's':
          log(va_arg(ap, const char*));
          break;

        // Decimal Integer
        case 'u':
          log(va_arg(ap, unsigned), 10);
          break;
        case 'd':
        case 'i':
          log(va_arg(ap, int), 10);
          break;

        // Hexadecimal Integer
        case 'x': // TODO: Implement lower case haxadecimal
        case 'X':
          log(va_arg(ap, unsigned), 16);
          break;

        // Long
        case 'l':
          switch(it[2])
          {
          case 'u':
            log(va_arg(ap, long unsigned), 10);
            break;
          case 'd':
          case 'i':
            log(va_arg(ap, long int), 10);
            break;

          // Hexadecimal Integer
          case 'x': // TODO: Implement lower case haxadecimal
          case 'X':
            log(va_arg(ap, long unsigned), 16);
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
}

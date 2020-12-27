#include <core/generic/io/Print.hpp>

#include <core/generic/devices/Framebuffer.hpp>

#include <limits.h>

#include <atomic>

namespace core::io
{
  volatile std::atomic<int> lock;
  void print(std::string_view str) 
  { 
    int expected = 0;
    if(lock.load() == 0 && lock.compare_exchange_strong(expected, 1))
    {
      devices::framebuffer.write(str); 
      lock.store(0);
    }
  }

  void print(unsigned value, unsigned base)
  {
    if(base==0 || base>16)
      return print("NaN");

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
      print(std::string_view(&buf[index], sizeof buf - index));
    else
      print("0");
  }

  void print(long unsigned value, unsigned base)
  {
    if(base==0 || base>16)
      return print("NaN");

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
      print(std::string_view(&buf[index], sizeof buf - index));
    else
      print("0");
  }

  void print(int value, unsigned base)
  {
    if(value>=0)
    {
      print(static_cast<unsigned>(value), base);
    }
    else
    {
      print("-");
      print(static_cast<unsigned>(-value), base);
    }
  }

  void print(long int value, unsigned base)
  {
    if(value>=0)
    {
      print(static_cast<long unsigned>(value), base);
    }
    else
    {
      print("-");
      print(static_cast<long unsigned>(-value), base);
    }
  }

  void printf(const char* format, ...)
  {
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
  }

  void vprintf(const char* format, va_list ap)
  {
    const char *begin = format, *it = format;
    auto flush = [&](){ print(std::string_view(begin, it)); };

    while(*it != 0)
    {
      if(*it=='%')
      {
        flush();
        switch(it[1])
        {
        case 's':
          print(va_arg(ap, const char*));
          break;

        // Decimal Integer
        case 'u':
          print(va_arg(ap, unsigned), 10);
          break;
        case 'd':
        case 'i':
          print(va_arg(ap, int), 10);
          break;

        // Hexadecimal Integer
        case 'x': // TODO: Implement lower case haxadecimal
        case 'X':
          print(va_arg(ap, unsigned), 16);
          break;

        // Long
        case 'l':
          switch(it[2])
          {
          case 'u':
            print(va_arg(ap, long unsigned), 10);
            break;
          case 'd':
          case 'i':
            print(va_arg(ap, long int), 10);
            break;

          // Hexadecimal Integer
          case 'x': // TODO: Implement lower case haxadecimal
          case 'X':
            print(va_arg(ap, long unsigned), 16);
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


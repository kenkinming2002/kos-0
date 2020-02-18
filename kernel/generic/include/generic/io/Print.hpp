#pragma once

#include <stddef.h>

namespace io
{
  /** 
   * print to screen according to format
   *
   * @param format the format string
   *
   * @return negative value upon error, number of characters printed otherwise.
   */
  int printf(const char* format, ...);

  /***********************************
   * Integer Formatting and Printing *
   ***********************************/
  enum class Base
  {
    BIN = 2,
    DEC = 10,
    OCT = 8,
    HEX = 16
  };

  template<typename T, Base base = Base::HEX>
  const char* toString(T value);
  template<typename T, Base base = Base::HEX>
  int printSingle(T value);

  /*******************
   * String Printing *
   *******************/
  int printSingle(const char* str);
  int printSingle(const char* str, size_t len);

  template<size_t N>
  int printSingle(const char(&str)[N])
  {
    return printSingle(str, N);
  }

  template<typename Arg>
  inline int print(const Arg& arg)
  {
    return printSingle(arg);
  }

  template<typename Arg, typename... Args>
  inline int print(const Arg& arg, const Args&... args)
  {
    int res = printSingle(arg);
    if(res==-1)
      return -1;
    else
      return res + print(args...);
  }
}

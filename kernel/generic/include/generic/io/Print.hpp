#pragma once

#include <generic/utils/Format.hpp>
#include <generic/io/Framebuffer.hpp>

#include <generic/io/Serial.hpp>

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

  template<typename... Args>
  int print(const Args&... args)
  {
    constexpr static size_t BUF_SIZE = 256;

    char buf[BUF_SIZE];
    {
      char* cur = buf;
      size_t capacity = BUF_SIZE;

      auto add = [&](const auto& arg){
        auto result = utils::format(cur, capacity, arg);
        if(result!=-1)
        {
          cur+=result;
          capacity-=result;
        }
      };
      (add(args), ...);

      io::frameBuffer.write(buf, BUF_SIZE-capacity);
      io::frameBuffer.write("\n", 1);
      io::com1Port.write(buf, BUF_SIZE-capacity);
      io::com1Port.write("\n", 1);

      return BUF_SIZE-capacity;
    }
  }
}

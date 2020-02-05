#pragma once

#include <utils/Format.hpp>
#include <io/Framebuffer.hpp>

#include <io/Serial.hpp>

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

  extern uint32_t lineNumber;

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

      //io::frameBuffer.write(io::FrameBuffer::Cursor{0u, lineNumber++}, buf,
      //    BUF_SIZE-capacity, io::FrameBuffer::Color::WHITE,
      //    io::FrameBuffer::Color::BLACK);
      serial_write(SERIAL_COM1_BASE, buf, BUF_SIZE-capacity);
      serial_write(SERIAL_COM1_BASE, "\n", 1);

      return BUF_SIZE-capacity;
    }
  }
}

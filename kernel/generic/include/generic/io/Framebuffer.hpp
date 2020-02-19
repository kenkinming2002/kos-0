#pragma once

#include <stdint.h>
#include <generic/grub/multiboot2.h>

#include <optional>

namespace io
{
  class FrameBuffer
  {
  public:
    enum class Color
    {
      BLACK         = 0,
      BLUE          = 1,
      GREEN         = 2,
      CYAN          = 3,
      RED           = 4,
      MAGENTA       = 5,
      BROWN         = 6,
      LIGHT_GREY    = 7,
      DARK_GREY     = 8,
      LIGHT_BLUE    = 9,
      LIGHT_GREEN   = 10,
      LIGHT_CYAN    = 11,
      LIGHT_RED     = 12,
      LIGHT_MAGENTA = 13,
      LIGHT_BROWN   = 14,
      WHITE         = 15
    };

    struct Cursor
    {
      size_t x, y;
    };

    struct Settings
    {
    };

  public:
    FrameBuffer();
    FrameBuffer(void* cells, size_t width, size_t height);

  public:
    int put(FrameBuffer::Cursor cursor, char c, FrameBuffer::Color fg, FrameBuffer::Color bg) const;
    void scroll();
    int write(const char* buf, size_t count);

  public:
    int setCursor(FrameBuffer::Cursor cursor) const;

  public:
    size_t width() const { return m_width; }
    size_t height() const { return m_height; }

  private:
    Cursor m_cursor;

  private:
    struct [[gnu::packed]] Cell
    {
    public:
      Cell(char c, Color fg, Color bg) 
        : c(c), color(static_cast<uint8_t>(bg) << 4 | static_cast<uint8_t>(fg)) {}

    public:
      char c;
      uint8_t color;
      // NOTE: This assume little endian
    };

  private:
    Cell* m_cells;
    size_t  m_width, m_height;
    FrameBuffer::Settings m_settings;
  };

  extern FrameBuffer frameBuffer;
}

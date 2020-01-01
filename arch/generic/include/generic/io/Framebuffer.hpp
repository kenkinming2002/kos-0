#pragma once

#include <stdint.h>
#include <generic/grub/multiboot2.h>

#include <generic/utils/Optional.hpp>

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
      uint32_t x, y;
    };

    struct Settings
    {
    };

  public:
    FrameBuffer() = default;
    FrameBuffer(uint16_t* cells, uint32_t width, uint32_t height);

  public:
    static utils::Optional<FrameBuffer> create(struct multiboot_tag_framebuffer *multiboot_tag_framebuffer);
    static utils::Optional<FrameBuffer> create();
    static FrameBuffer createDefault();

  public:
    int write(FrameBuffer::Cursor cursor, char c, FrameBuffer::Color fg, FrameBuffer::Color bg) const;
    int write(FrameBuffer::Cursor cursor, const char* buf, uint32_t len, FrameBuffer::Color fg, FrameBuffer::Color bg) const;

  public:
    int setCursor(FrameBuffer::Cursor cursor) const;

  private:
    uint16_t* m_cells;
    uint32_t  m_width, m_height;
    FrameBuffer::Settings m_settings;
  };

  extern FrameBuffer frameBuffer;

  namespace framebuffer
  {
    int init();
  }
}

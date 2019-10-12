#ifndef FRAMBUFFER_H
#define FRAMBUFFER_H

#include <stdint.h>
#include <grub/multiboot2.h>

#include <utils/Optional.hpp>

enum class FrameBufferColor
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

struct FrameBufferCursor
{
  uint32_t x, y;
};

struct FrameBufferSettings
{
};

class FrameBuffer
{
public:
  FrameBuffer() = default;
  FrameBuffer(uint16_t* cells, uint32_t width, uint32_t height);

public:
  static utils::Optional<FrameBuffer> create(struct multiboot_tag_framebuffer *multiboot_tag_framebuffer);
  static FrameBuffer createDefault();

public:
  int write(FrameBufferCursor cursor, char c, FrameBufferColor fg, FrameBufferColor bg) const;
  int write(FrameBufferCursor cursor, const char* buf, uint32_t len, FrameBufferColor fg, FrameBufferColor bg) const;

public:
  int setCursor(FrameBufferCursor cursor) const;

private:
  uint16_t* m_cells;
  uint32_t  m_width, m_height;
  FrameBufferSettings m_settings;
};

#endif // FRAMBUFFER_H

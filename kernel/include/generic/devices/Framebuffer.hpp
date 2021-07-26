#pragma once

#include <stddef.h>
#include <stdint.h>

namespace core::devices
{
  class Framebuffer
  {
  public:
    static void initializeLog();

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

  private:
    struct [[gnu::packed]] Cell
    {
    public:
      Cell(char c, Color fg, Color bg)
        : c(c), color(static_cast<uint8_t>(bg) << 4 | static_cast<uint8_t>(fg)) {}

    public:
      // NOTE: This assume little endian
      char c;
      uint8_t color;
    };

  public:
    Framebuffer();

  private:
    void put(char c) const;
    void newline();
    void scroll();
    void updateCursor() const;

  public:
    int write(const char* str, size_t length);

  public:
    size_t width() const { return m_width; }
    size_t height() const { return m_height; }

  private:
    size_t m_x=0, m_y=0;
    Color m_fg=Color::WHITE, m_bg=Color::BLACK;

  private:
    Cell* m_cells;
    size_t  m_width, m_height;
  };

}

#include "librt/Optional.hpp"
#include <generic/devices/Framebuffer.hpp>

#include <generic/BootInformation.hpp>

#include <x86/assembly/io.hpp>

#include <librt/Global.hpp>
#include <librt/Algorithm.hpp>

namespace core::devices
{
  namespace
  {
    constinit rt::Optional<Framebuffer> framebuffer;
  }

  Framebuffer& Framebuffer::instance()
  {
    if(!framebuffer)
      framebuffer = Framebuffer();

    return *framebuffer;
  }

  namespace
  {
    /* The I/O ports */
    constexpr uint16_t FB_COMMAND_PORT = 0x3D4;
    constexpr uint16_t FB_DATA_PORT    = 0x3D5;

    /* The I/O port commands */
    constexpr uint16_t FB_HIGH_BYTE_COMMAND = 14;
    constexpr uint16_t FB_LOW_BYTE_COMMAND  = 15;
  }

  Framebuffer::Framebuffer() : m_cells(nullptr), m_width(0), m_height(0)
  {
    m_cells = reinterpret_cast<Cell*>(bootInformation->framebuffer);
    m_width = 80;
    m_height = 25;
  }

  void Framebuffer::put(char c) const
  {
    if(!m_cells) return;
    if(m_y>=m_height || m_x>=m_width) return;
    m_cells[m_y * m_width + m_x] = Cell(c, m_fg, m_bg);
  }

  void Framebuffer::newline()
  {
    m_x = 0;
    ++m_y;
    if(m_y==m_height)
      scroll();
  }

  void Framebuffer::scroll()
  {
    --m_y;
    rt::copy(&m_cells[m_width], &m_cells[m_width*m_height], m_cells);
    rt::fill(&m_cells[m_width*(m_height-1)], &m_cells[m_width*m_height], Cell(' ', m_fg, m_bg));
  }

  void Framebuffer::updateCursor() const
  {
    size_t pos = m_y * m_width + m_x;

    assembly::outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    assembly::outb(FB_DATA_PORT, (pos >> 8) & 0x00FF);
    assembly::outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    assembly::outb(FB_DATA_PORT, pos & 0x00FF);
  }

  int Framebuffer::write(const char* str, size_t length)
  {
    for(size_t i=0; i<length; ++i)
    {
      const auto c = str[i];
      switch(c)
      {
      case '\n':
        newline();
        break;
      case '\r':
        m_x=0;
        break;
      case '\b':
        if(m_x!=0)
        {
          --m_x;
          put(' ');
        }
        break;
      default:
        put(c);
        ++m_x;
        if(m_x == m_width)
          newline();
        break;
      }
    }
    updateCursor();
    return length;
  }
}

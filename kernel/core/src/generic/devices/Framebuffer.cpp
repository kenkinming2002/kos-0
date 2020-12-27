#include <core/generic/devices/Framebuffer.hpp>

#include <core/generic/Init.hpp>

#include <boot/i686/BootInformation.hpp>
#include <core/x86/assembly/io.hpp>

#include <algorithm>

namespace core::devices
{
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
    auto& multiboot_tag_framebuffer = BOOT_INFORMATION.frameBuffer;

    if(multiboot_tag_framebuffer.common.framebuffer_type!=2) 
      return;
    // The highest 32 bit is not all zero. The address is not an 32-bit address, and require PAE, which we do not support
    if(multiboot_tag_framebuffer.common.framebuffer_addr >> 32) 
      return;

    m_cells = reinterpret_cast<Cell*>(static_cast<uintptr_t>(multiboot_tag_framebuffer.common.framebuffer_addr));
    m_width = multiboot_tag_framebuffer.common.framebuffer_width;
    m_height = multiboot_tag_framebuffer.common.framebuffer_height;
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
    std::copy(&m_cells[m_width], &m_cells[m_width*m_height], m_cells);
    std::fill(&m_cells[m_width*(m_height-1)], &m_cells[m_width*m_height], Cell(' ', m_fg, m_bg));
  }

  void Framebuffer::updateCursor() const
  {
    size_t pos = m_y * m_width + m_x;

    assembly::outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    assembly::outb(FB_DATA_PORT, (pos >> 8) & 0x00FF);
    assembly::outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    assembly::outb(FB_DATA_PORT, pos & 0x00FF);
  }

  int Framebuffer::write(std::string_view str)
  {
    for(char c : str)
    {
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
    return str.length();
  }

  INIT_NOLOG Framebuffer framebuffer;
}

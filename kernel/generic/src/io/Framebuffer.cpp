#include <generic/io/Framebuffer.hpp>

#include <generic/io/Serial.hpp>

#include <intel/asm/io.hpp>
#include <i686/boot/boot.hpp>

namespace io
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

  FrameBuffer::FrameBuffer() : m_cells(nullptr), m_width(0), m_height(0)
  {
    auto& multiboot_tag_framebuffer = utils::deref_cast<BootInformation>(bootInformationStorage).frameBuffer;

    if(multiboot_tag_framebuffer.common.framebuffer_type!=2) 
      return;
    // The highest 32 bit is not all zero. The address is not an 32-bit address, and require PAE, which we do not support
    if(multiboot_tag_framebuffer.common.framebuffer_addr >> 32) 
      return;

    m_cells = reinterpret_cast<Cell*>(static_cast<uintptr_t>(multiboot_tag_framebuffer.common.framebuffer_addr) + 0xC0000000);
    m_width = multiboot_tag_framebuffer.common.framebuffer_width;
    m_height = multiboot_tag_framebuffer.common.framebuffer_height;
  }

  FrameBuffer::FrameBuffer(void* cells, size_t width, size_t height)
    : m_cells(static_cast<Cell*>(cells)), m_width(width), m_height(height) {}

  int FrameBuffer::put(FrameBuffer::Cursor cursor, char c, FrameBuffer::Color fg, FrameBuffer::Color bg) const
  {
    if(cursor.y>=m_height || cursor.x>=m_width)
      return -1;

    size_t i = cursor.y * m_width + cursor.x;
    m_cells[i] = Cell(c, fg, bg);

    return 0;
  }

  void FrameBuffer::scroll()
  {
    --m_cursor.y;
    std::copy(&m_cells[m_width], &m_cells[m_width*m_height], m_cells);
    std::fill(&m_cells[m_width*(m_height-1)], &m_cells[m_width*m_height], Cell(' ', Color::WHITE, Color::BLACK));
  }

  int FrameBuffer::write(const char* buf, size_t count)
  {
    for(size_t i=0; i<count; ++i)
    {
      switch(buf[i])
      {
      case '\n':
      {
        m_cursor = {0, m_cursor.y+1};
        break;
      }
      case '\r':
        m_cursor.x = 0;
        break;
      case '\b':
        if(m_cursor.x!=0)
        {
          --m_cursor.x;
          put(m_cursor, ' ', FrameBuffer::Color::WHITE, FrameBuffer::Color::BLACK);
        }
        break;
      default:
        put(m_cursor, buf[i], FrameBuffer::Color::WHITE, FrameBuffer::Color::BLACK);
        ++m_cursor.x;
      }

      if(m_cursor.x == m_width)
        m_cursor = {0, m_cursor.y+1};
      while(m_cursor.y >= m_height)
        this->scroll(); // NOTE: Maybe optimize this
    }

    this->setCursor(m_cursor);

    return count;
  }

  int FrameBuffer::setCursor(FrameBuffer::Cursor cursor) const
  {
    size_t pos = cursor.y * m_width + cursor.x;

    assembly::outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    assembly::outb(FB_DATA_PORT, (pos >> 8) & 0x00FF);
    assembly::outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    assembly::outb(FB_DATA_PORT, pos & 0x00FF);

    return 0;
  }

  __attribute__((init_priority(101))) FrameBuffer frameBuffer;
}

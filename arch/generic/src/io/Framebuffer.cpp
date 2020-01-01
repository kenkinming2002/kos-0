#include <generic/io/Framebuffer.hpp>

extern "C" {
#include <generic/io/serial.h>
}

#include <intel/asm/io.hpp>
#include <boot/lower_half.hpp>

namespace
{
  bool fb_created = false;
}


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

  FrameBuffer::FrameBuffer(uint16_t* cells, uint32_t width, uint32_t height)
    : m_cells(cells), m_width(width), m_height(height) {}

  utils::Optional<FrameBuffer> FrameBuffer::create()
  {
    return create(&bootInformation.framebuffer);
  }

  utils::Optional<FrameBuffer> FrameBuffer::create(struct multiboot_tag_framebuffer *multiboot_tag_framebuffer)
  {
    if(multiboot_tag_framebuffer->common.framebuffer_type!=2)
      return utils::nullopt; // Framebuffer is not in EGA text mode

    // The highest 32 bit is not all zero. The address is not an 32-bit address, and require PAE, which we do not support
    if(multiboot_tag_framebuffer->common.framebuffer_addr >> 32) return utils::nullopt; 

    return FrameBuffer(
      reinterpret_cast<uint16_t*>(static_cast<uint32_t>(multiboot_tag_framebuffer->common.framebuffer_addr) + 0xC0000000),
      multiboot_tag_framebuffer->common.framebuffer_width,
      multiboot_tag_framebuffer->common.framebuffer_height
    );
  }

  FrameBuffer FrameBuffer::createDefault()
  {
    return FrameBuffer(reinterpret_cast<uint16_t*>(0xC00B8000), 80u, 60u);
  }

  int FrameBuffer::write(FrameBuffer::Cursor cursor, char c, FrameBuffer::Color fg, FrameBuffer::Color bg) const
  {
    if(cursor.y>=m_height || cursor.x>=m_width)
      return -1;

    uint32_t i = cursor.y * m_width + cursor.x;
    uint16_t color = static_cast<uint16_t>(bg) << 4 | static_cast<uint16_t>(fg);
    m_cells[i] = color << 8 | c;

    return 0;
  }

  int FrameBuffer::write(FrameBuffer::Cursor cursor, const char* buf, uint32_t len, FrameBuffer::Color fg, FrameBuffer::Color bg) const
  {
    for(unsigned int i=0; i<len; i++)
    {
      switch(buf[i])
      {
      case '\n':
        ++cursor.y;
          if(cursor.y==m_width)
            return -1;//TODO: scroll the buffer
        break;
      default:
        write(cursor, buf[i], fg, bg);

        ++cursor.x;
        if(cursor.x==m_width)
        {
          cursor.x = 0;
          ++cursor.y;
          if(cursor.y==m_width)
            return -1;//TODO: scroll the buffer
        }
        break;
      }
    }

    return len;
  }

  int FrameBuffer::setCursor(FrameBuffer::Cursor cursor) const
  {
    uint32_t pos = cursor.y * m_width + cursor.x;
    assembly::outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    assembly::outb(FB_DATA_PORT, (pos >> 8) & 0x00FF);
    assembly::outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    assembly::outb(FB_DATA_PORT, pos & 0x00FF);
    return 0;
  }

  FrameBuffer frameBuffer;

  namespace framebuffer
  {
    int init()
    {
      auto fb = FrameBuffer::create();
      if(fb)
        frameBuffer = *fb;
      else
        frameBuffer = FrameBuffer::createDefault();
      return 0;
    }
  }
}

//namespace init::multiboot2
//{
//  int parse_framebuffer_tag(struct multiboot_tag_framebuffer *framebuffer_tag)
//  {
//      if(!fb_created)
//      {
//        auto frameBuffer = io::FrameBuffer::create(framebuffer_tag);
//        if(frameBuffer)
//        {
//          io::frameBuffer = *frameBuffer;
//          fb_created = true;
//        }
//      }
//
//      return 0;
//  }
//}
//

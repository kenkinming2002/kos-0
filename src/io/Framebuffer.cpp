#include <io/Framebuffer.hpp>

#include <io/serial.h>

#include <asm/io.h>

/* The I/O ports */
#define FB_COMMAND_PORT         0x3D4
#define FB_DATA_PORT            0x3D5

/* The I/O port commands */
#define FB_HIGH_BYTE_COMMAND    14
#define FB_LOW_BYTE_COMMAND     15

FrameBuffer::FrameBuffer(uint16_t* cells = reinterpret_cast<uint16_t*>(0xC00B8000), uint32_t width = 80, uint32_t height = 60)
  : m_cells(cells), m_width(width), m_height(height) {}

utils::Optional<FrameBuffer> FrameBuffer::create(struct multiboot_tag_framebuffer *multiboot_tag_framebuffer)
{
  if(multiboot_tag_framebuffer->common.framebuffer_type!=2)
    return utils::nullopt; // Framebuffer is not in EGA text mode

  if(multiboot_tag_framebuffer->common.framebuffer_addr >> 32)
    return utils::nullopt; // The highest 32 bit is not all zero. The address is not an 32-bit address, and require PAE, which we do not support

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

int FrameBuffer::write(FrameBufferCursor cursor, char c, FrameBufferColor fg, FrameBufferColor bg) const
{
  if(cursor.y>=m_height || cursor.x>=m_width)
    return -1;

  uint32_t i = cursor.y * m_width + cursor.x;
  uint16_t color = static_cast<uint16_t>(bg) << 4 | static_cast<uint16_t>(fg);
  m_cells[i] = color << 8 | c;

  return 0;
}

int FrameBuffer::write(FrameBufferCursor cursor, const char* buf, uint32_t len, FrameBufferColor fg, FrameBufferColor bg) const
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



//static struct fb* fb = (struct fb*)0x000B8000;

//int fb_create(fb_t *fb, struct multiboot_tag_framebuffer* multiboot_tag_framebuffer)
//{
//
//  if(multiboot_tag_framebuffer->common.framebuffer_type!=2)
//    return -1; // Framebuffer is not in EGA text mode
//
//  if(multiboot_tag_framebuffer->common.framebuffer_addr >> 32)
//    return -1; // The highest 32 bit is not all zero. The address is not an 32-bit address, but require PAE.
//  fb->cells  = (char*)(uint32_t)multiboot_tag_framebuffer->common.framebuffer_addr + 0xC0000000; // Translate from physical to virtual address
//  fb->width  = multiboot_tag_framebuffer->common.framebuffer_width; 
//  fb->height = multiboot_tag_framebuffer->common.framebuffer_height;
//
//
//  return 0;
//}
//
//int fb_create_default(fb_t *fb)
//{
//  fb->cells = (unsigned *)0xC00B8000;
//  fb->width = 80;
//  fb->height = 60;
//
//  return 0;
//}
//
//int fb_write_cell(fb_t *fb, fb_cursor_t cursor, char c, fb_color_t fg, fb_color_t bg)
//{
//  if(cursor.y>=fb->height || cursor.x>=fb->width)
//    return -1;
//
//  uint32_t i = cursor.y * fb->width + cursor.x;
//  uint16_t color = bg << 4 | fg;
//  fb->cells[i] = color << 8 | c;
//
//  return 0;
//}
//
//int fb_set_cursor(fb_t *fb, fb_cursor_t cursor)
//{
//  uint32_t pos = cursor.y * fb->width + cursor.x;
//  outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
//  outb(FB_DATA_PORT, (pos >> 8) & 0x00FF);
//  outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
//  outb(FB_DATA_PORT, pos & 0x00FF);
//
//  return 0;
//}
//
//int fb_write(fb_t *fb, fb_cursor_t cursor, const char* buf, unsigned int len, unsigned char fg, unsigned char bg)
//{
//  for(unsigned int i=0; i<len; i++)
//  {
//    switch(buf[i])
//    {
//    case '\n':
//      ++cursor.y;
//        if(cursor.y==fb->width)
//          return -1;//TODO: scroll the buffer
//      break;
//    default:
//      fb_write_cell(fb, cursor, buf[i], fg, bg);
//
//      ++cursor.x;
//      if(cursor.x==fb->width)
//      {
//        cursor.x = 0;
//        ++cursor.y;
//        if(cursor.y==fb->width)
//          return -1;//TODO: scroll the buffer
//      }
//      break;
//    }
//  }
//
//  return len;
//}

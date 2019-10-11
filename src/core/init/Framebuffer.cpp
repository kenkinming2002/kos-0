#include <core/init/Framebuffer.hpp>

FrameBuffer g_frameBuffer;
bool fb_created = false;

int multiboot2_tag_framebuffer_parse(struct multiboot_tag_framebuffer *framebuffer_tag)
{
    if(!fb_created)
    {
      auto frameBuffer = FrameBuffer::create(framebuffer_tag);
      if(frameBuffer)
      {
        g_frameBuffer = *frameBuffer;
        fb_created = true;
      }
    }

    return 0;
}

int framebuffer_init()
{
  if(!fb_created)
    g_frameBuffer = FrameBuffer::createDefault();

  return 0;
}

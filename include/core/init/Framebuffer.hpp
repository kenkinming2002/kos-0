#pragma once

#include <io/Framebuffer.hpp>

extern FrameBuffer g_frameBuffer;

namespace init::multiboot2
{
  int parse_framebuffer_tag(struct multiboot_tag_framebuffer *framebuffer_tag);
}

int framebuffer_init();

#pragma once

#include <io/Framebuffer.hpp>

extern FrameBuffer g_frameBuffer;

int multiboot2_tag_framebuffer_parse(struct multiboot_tag_framebuffer *framebuffer_tag);

int framebuffer_init();

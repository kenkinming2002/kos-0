#include <generic/log/Log.hpp>

#include <generic/devices/Framebuffer.hpp>

namespace core::log
{
  namespace
  {
    devices::Framebuffer* framebuffer;
  }

  void initialize()
  {
    framebuffer = &devices::Framebuffer::instance();
  }
}

namespace rt::hooks
{
  void log(const char* str, size_t length)
  {
    core::log::framebuffer->write(str, length);
  }
}


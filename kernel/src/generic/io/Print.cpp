#include <common/generic/io/Print.hpp>

#include <generic/devices/Framebuffer.hpp>

#include <atomic>

namespace common::io
{
  using namespace core;
  void print(std::string_view str) 
  { 
    devices::Framebuffer::instance().write(str); 
  }
}


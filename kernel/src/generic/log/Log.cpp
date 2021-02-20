#include <generic/log/Log.hpp>

#include <generic/devices/Framebuffer.hpp>

namespace core::log
{
  namespace
  {
    Logger logger;
  }

  void initialize()
  {
    devices::Framebuffer::initializeLog();
  }

  void registerLogger(Logger logger)
  {
    core::log::logger = logger;
  }
}

namespace rt::hooks
{
  void log(const char* str, size_t length)
  {
    core::log::logger(str, length);
  }
}


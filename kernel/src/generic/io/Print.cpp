#include <common/generic/io/Print.hpp>

#include <generic/devices/Framebuffer.hpp>

#include <atomic>

namespace common::io
{
  using namespace core;
  volatile std::atomic<int> lock;
  void print(std::string_view str) 
  { 
    int expected = 0;
    if(lock.load() == 0 && lock.compare_exchange_strong(expected, 1))
    {
      devices::framebuffer.write(str); 
      lock.store(0);
    }
  }
}


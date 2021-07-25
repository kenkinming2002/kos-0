#include <generic/memory/Memory.hpp>

#include <generic/memory/Syscalls.hpp>

namespace core::memory
{
  void initialize()
  {
    initializeHeap();
    initializeVirtual();
    initializeSyscalls();
  }
}

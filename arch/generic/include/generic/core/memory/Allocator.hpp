#pragma once

#include <stddef.h>

namespace core::memory
{
  template<typename T, typename PhysicalAllocator, typename VirtualAllocator>
  class Allocator
  {
  public:
    T* allocate(size_t n) {}
    void deallocate(T* t, size_t n) {}

    template<typename... Args>
    void construct(T*, Args&&...) {}

    template<typename U>
    using rebind = Allocator<U, PhysicalAllocator, VirtualAllocator>;

  private:
    //PhysicalAllocator& m_physicalAllocator;
    //VirtualAllocator& m_virtualAllocator;
  };

  class PhysicalAllocator;
  class BootPhysicalAllocator;

  class VirtualAllocator;
  class BootVirtualAllocator;

  template<typename T>
  using DefaultAllocator = Allocator<T, PhysicalAllocator, VirtualAllocator>;
  template<typename T>
  using BootAllocator = Allocator<T, BootPhysicalAllocator, BootVirtualAllocator>;
}


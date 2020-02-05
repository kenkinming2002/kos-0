#pragma once

#include <stddef.h>

#include <liballoc_1_1.h>

#include <io/Print.hpp>

namespace allocators
{
  template<typename T>
  class CAllocator
  {
  public:
    typedef T*          pointer;
    typedef const T*    const_pointer;
    typedef void*       void_pointer;
    typedef const void* const_void_pointer;
    typedef T           value_type;
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    template<typename U>
    struct rebind
    {
      using other = CAllocator<U>;
    };

    pointer allocate(size_type n) noexcept
    {
      auto mem = kmalloc(n * sizeof(T));
      io::print("MARKER-malloc ", reinterpret_cast<uintptr_t>(mem));
      return static_cast<pointer>(mem);
    }

    void deallocate(pointer ptr, size_type /*n*/) noexcept
    {
      kfree(ptr);
    }
  };
}

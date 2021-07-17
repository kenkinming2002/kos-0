#pragma once

#include <generic/memory/Memory.hpp>
#include <i686/memory/Memory.hpp>

#include <librt/SharedPtr.hpp>
#include <librt/UniquePtr.hpp>
#include <librt/NonCopyable.hpp>
#include <librt/Panic.hpp>
#include <librt/Log.hpp>

#include <stdint.h>
#include <stddef.h>

namespace core::memory
{
  class Page : public rt::SharedPtrHook
  {
  public:
    static rt::SharedPtr<Page> allocate()
    {
      void* ptr = allocPages(1);
      if(!ptr)
        return nullptr;

      return rt::makeShared<Page>(ptr);
    }

  public:
    constexpr Page(void* ptr) : ptr(ptr) {}
    ~Page() { if(ptr) freePages(ptr, 1); }

  public:
    uintptr_t virtaddr() const { return reinterpret_cast<uintptr_t>(ptr); }
    physaddr_t physaddr() const { return virtToPhys(virtaddr()); }

  public:
    void* ptr;

  public:
    /* TODO: spinlock? */
    bool dirty    : 1 = false; /* page mapped read-write are dirty */
    bool cow      : 1 = false; /* cow page must be mapped read-only */
  };
}

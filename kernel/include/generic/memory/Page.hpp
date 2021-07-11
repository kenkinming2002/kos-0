#pragma once

#include <generic/memory/Memory.hpp>

#include <librt/SharedPtr.hpp>
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
      void* page = allocPages(1);
      if(!page)
        return nullptr;

      return rt::makeShared<Page>(page);
    }

  public:
    constexpr Page(void* ptr) : ptr(ptr) {}
    ~Page() { if(ptr) freePages(ptr, 1); }

  public:
    void* ptr;

  public:
    /* TODO: spinlock? */
    bool dirty    : 1 = false; /* page mapped read-write are dirty */
    bool cow      : 1 = false; /* cow page must be mapped read-only */
    bool writable : 1 = false;
  };
}

#pragma once

#include "librt/SharedPtr.hpp"
#include <generic/memory/Memory.hpp>

#include <librt/NonCopyable.hpp>
#include <librt/Panic.hpp>
#include <librt/Log.hpp>

#include <stdint.h>
#include <stddef.h>

namespace core::memory
{
  class Pages : public rt::SharedPtrHook
  {
  public:
    static rt::SharedPtr<Pages> allocate(size_t count)
    {
      void* page = allocPages(count);
      if(!page)
        return nullptr;

      return rt::makeShared<Pages>(page, count);
    }

  public:
    constexpr Pages(void* page, size_t count) : m_page(page), m_count(count) {}
    ~Pages() { if(m_page) freePages(m_page, m_count); }

  public:
    void* ptr() { return m_page; }

  private:
    void* m_page;
    size_t m_count;
  };
}

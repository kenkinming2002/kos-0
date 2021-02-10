#pragma once

#include <generic/memory/Pages.hpp>

#include <librt/Optional.hpp>
#include <librt/Assert.hpp>

namespace core::memory
{
  class BootPagesAllocator
  {
  public:
    constexpr BootPagesAllocator() = default;

    rt::Optional<Pages> allocMappedPages(size_t count)
    {
      ASSERT(!m_used && "Early Heap already used but the memory system is not yet initialized");
      ASSERT(count == 16 && "Early heap size mismatch");

      return Pages::from(reinterpret_cast<uintptr_t>(m_earlyHeap), sizeof m_earlyHeap);
    }


  private:
    alignas(PAGE_SIZE) char m_earlyHeap[PAGE_SIZE * 16] = {};
    bool m_used = false;
  };
}


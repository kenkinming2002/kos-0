#pragma once

#include <generic/memory/Pages.hpp>

#include <optional>
#include <assert.h>

namespace core::memory
{
  class BootPagesAllocator
  {
  public:
    constexpr BootPagesAllocator() = default;

    std::optional<Pages> allocMappedPages(size_t count)
    {
      assert(!m_used && "Early Heap already used but the memory system is not yet initialized");
      assert(count == 16 && "Early heap size mismatch");

      return Pages::from(reinterpret_cast<uintptr_t>(m_earlyHeap), sizeof m_earlyHeap);
    }


  private:
    alignas(PAGE_SIZE) char m_earlyHeap[PAGE_SIZE * 16] = {};
    bool m_used = false;
  };
}


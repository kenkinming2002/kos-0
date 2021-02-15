#include <generic/memory/Virtual.hpp>

#include <librt/Log.hpp>
#include <librt/Optional.hpp>
#include <librt/containers/Bitset.hpp>

namespace core::memory
{
  namespace
  {
    constexpr uintptr_t HEAP_START = 0xD0000000;
    constexpr uintptr_t HEAP_END   = 0xE0000000;
    constexpr uintptr_t HEAP_START_INDEX = HEAP_START / PAGE_SIZE;
    constexpr uintptr_t HEAP_END_INDEX   = HEAP_END / PAGE_SIZE;
    constexpr uintptr_t HEAP_PAGES_COUNT = HEAP_END_INDEX - HEAP_START_INDEX;

    rt::containers::Bitset<HEAP_PAGES_COUNT> bitset;

    bool get(size_t index)
    {
      return bitset.get(index - HEAP_START_INDEX);
    }

    void markRegion(size_t index, size_t count, bool value)
    {
      for(size_t i=index; i<index+count; ++i)
        bitset.set(i - HEAP_START_INDEX, value);
    }
  }

  void initializeVirtual()
  {
    // Nothing to do, the bitset is all empty
  }

  rt::Optional<Pages> allocVirtualPages(size_t count)
  {
    size_t realCount = 0;
    for(size_t i=HEAP_START_INDEX; i<HEAP_END_INDEX; ++i)
    {
      realCount = get(i) ? 0 : realCount+1;
      if(realCount == count)
      {
        markRegion(i - (count-1) , count, true);
        return Pages{.index = i - (count-1), .count = count};
      }
    }

    return rt::nullOptional;
  }

  void freeVirtualPages(Pages pages)
  {
    markRegion(pages.index, pages.count, false);
  }
}

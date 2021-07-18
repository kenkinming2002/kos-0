#pragma once

#include <generic/BootInformation.hpp>
#include <generic/memory/Memory.hpp>
#include <generic/memory/Memory.hpp>

#include <librt/containers/StaticVector.hpp>
#include <librt/Pair.hpp>

namespace core::memory
{
  static constexpr size_t MAX_MEMORY_REGIONS_COUNT = 32;

  struct MemoryRegion
  {
    uintptr_t addr;
    size_t length;

    static constexpr MemoryRegion from(uintptr_t begin, uintptr_t end)
    {
      uintptr_t addr = begin;
      uintptr_t length = end>=begin ? end-begin : 0;
      return MemoryRegion{addr, length};
    }

    constexpr uintptr_t begin() const { return addr; }
    constexpr uintptr_t end()   const { return addr+length; }
    constexpr bool empty() const { return length == 0; }

    static constexpr bool overlap(MemoryRegion lhs, MemoryRegion rhs) { return lhs.begin()<rhs.end() && rhs.begin()<lhs.end(); }
    static constexpr MemoryRegion merge(MemoryRegion lhs, MemoryRegion rhs) { return MemoryRegion::from(rt::min(lhs.begin(), rhs.begin()), rt::max(lhs.end(), rhs.end())); } // Precondition: overlap(lhs, rhs) == true

    // Precondition: overlap(lhs, rhs) == true
    constexpr rt::Pair<MemoryRegion, MemoryRegion> splitBy(MemoryRegion memoryRegion) const
    {
      auto front = MemoryRegion::from(this->begin(), memoryRegion.begin());
      auto back  = MemoryRegion::from(memoryRegion.end(), this->end());
      return {front, back};
    }
  };

  inline void add(rt::containers::StaticVector<MemoryRegion, MAX_MEMORY_REGIONS_COUNT>& memoryRegions, MemoryRegion memoryRegion) { memoryRegions.pushBack(memoryRegion); }
  inline void remove(rt::containers::StaticVector<MemoryRegion, MAX_MEMORY_REGIONS_COUNT>& memoryRegions, MemoryRegion memoryRegion)
  {
    rt::containers::StaticVector<MemoryRegion, MAX_MEMORY_REGIONS_COUNT> newMemoryRegions;
    for(const auto& existingMemoryRegion : memoryRegions)
      if(MemoryRegion::overlap(existingMemoryRegion, memoryRegion))
      {
        const auto [first, second] = existingMemoryRegion.splitBy(memoryRegion);
        if(!first.empty())
          newMemoryRegions.pushBack(first);
        if(!second.empty())
          newMemoryRegions.pushBack(second);

        // The memory region to remove may collide with multiple existing memory
        // region so we can not bail out early.
      }
      else
        newMemoryRegions.pushBack(existingMemoryRegion);

    memoryRegions = rt::move(newMemoryRegions);
  }

  constexpr uintptr_t roundDown(uintptr_t addr, size_t alignment) { return addr / alignment * alignment; }
  constexpr uintptr_t roundUp(uintptr_t addr, size_t alignment) { return roundDown(addr + alignment - 1, alignment); }

  inline void align(rt::containers::StaticVector<MemoryRegion, MAX_MEMORY_REGIONS_COUNT>& memoryRegions)
  {
    rt::containers::StaticVector<MemoryRegion, MAX_MEMORY_REGIONS_COUNT> newMemoryRegions;
    for(auto& existingMemoryRegion : memoryRegions)
    {
      auto begin = roundUp(existingMemoryRegion.begin(), PAGE_SIZE);
      auto end   = roundDown(existingMemoryRegion.end(), PAGE_SIZE);
      auto newMemoryRegion = MemoryRegion::from(begin, end);
      if(!newMemoryRegion.empty())
        newMemoryRegions.pushBack(newMemoryRegion);
    }
    memoryRegions = newMemoryRegions;
  }

  inline void sort(rt::containers::StaticVector<MemoryRegion, MAX_MEMORY_REGIONS_COUNT>& memoryRegions)
  {
    rt::sort(memoryRegions.begin(), memoryRegions.end(), [](const MemoryRegion& lhs, const MemoryRegion& rhs) { return lhs.addr<=rhs.addr; });
  }

  inline void merge(rt::containers::StaticVector<MemoryRegion, MAX_MEMORY_REGIONS_COUNT>& memoryRegions)
  {
    memoryRegions.erase(rt::adjacentMerge(memoryRegions.begin(), memoryRegions.end(), &MemoryRegion::overlap, &MemoryRegion::merge), memoryRegions.end());
  }

  inline void sanitize(rt::containers::StaticVector<MemoryRegion, MAX_MEMORY_REGIONS_COUNT>& memoryRegions)
  {
    align(memoryRegions);
    sort(memoryRegions);
    merge(memoryRegions);
  }
}

#include <generic/memory/Physical.hpp>

#include <generic/BootInformation.hpp>
#include <i686/memory/MemoryMapping.hpp>

#include <librt/Panic.hpp>
#include <librt/Log.hpp>
#include <librt/Assert.hpp>
#include <librt/Global.hpp>
#include <librt/Pair.hpp>
#include <librt/Algorithm.hpp>
#include <librt/containers/StaticVector.hpp>

#include <new>
#include <type_traits>

namespace core::memory
{
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

    constexpr static bool overlap(MemoryRegion lhs, MemoryRegion rhs)
    {
      return lhs.begin()<rhs.end() && rhs.begin()<lhs.end();
    }

    // Precondition: overlap(lhs, rhs) == true
    static constexpr MemoryRegion merge(MemoryRegion lhs, MemoryRegion rhs)
    {
      return MemoryRegion::from(rt::min(lhs.begin(), rhs.begin()), rt::max(lhs.end(), rhs.end()));
    }

    // Precondition: overlap(lhs, rhs) == true
    constexpr rt::Pair<MemoryRegion, MemoryRegion> splitBy(MemoryRegion memoryRegion) const
    {
      auto front = MemoryRegion::from(this->begin(), memoryRegion.begin());
      auto back  = MemoryRegion::from(memoryRegion.end(), this->end());
      return {front, back};
    }
  };

  static void add(rt::StaticVector<MemoryRegion, 32>& memoryRegions, MemoryRegion memoryRegion) { memoryRegions.pushBack(memoryRegion); }
  static void remove(rt::StaticVector<MemoryRegion, 32>& memoryRegions, MemoryRegion memoryRegion)
  {
    rt::StaticVector<MemoryRegion, 32> newMemoryRegions; // We need to be careful of stack usage
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

  static void sanitize(rt::StaticVector<MemoryRegion, 32>& memoryRegions)
  {
    rt::sort(memoryRegions.begin(), memoryRegions.end(), [](const MemoryRegion& lhs, const MemoryRegion& rhs) { return lhs.addr<=rhs.addr; });

    // We shouldn't actually need to do this step if bootloader hand us a sane
    // memory map which does not have any overlapping region
    memoryRegions.erase(rt::adjacentMerge(memoryRegions.begin(), memoryRegions.end(), &MemoryRegion::overlap, &MemoryRegion::merge), memoryRegions.end());
  }

  static rt::StaticVector<MemoryRegion, 32> prepare()
  {
    rt::StaticVector<MemoryRegion, 32> memoryRegions;
    for(size_t i=0; i<bootInformation->memoryMapEntriesCount; ++i)
      if(bootInformation->memoryMapEntries[i].type == MemoryMapEntry::Type::AVAILABLE)
      {
        const auto& memoryMapEntries = bootInformation->memoryMapEntries[i];
        add(memoryRegions, MemoryRegion{memoryMapEntries.addr, memoryMapEntries.len});
      }

    for(size_t i=0; i<bootInformation->moduleEntriesCount; ++i)
    {
      const auto& moduleEntries = bootInformation->moduleEntries[i];
      remove(memoryRegions, MemoryRegion{moduleEntries.addr, moduleEntries.len});
    }

    for(size_t i=0; i<bootInformation->reservedMemoryRegionsCount; ++i)
    {
      const auto& reservedMemoryRegions = bootInformation->reservedMemoryRegions[i];
      remove(memoryRegions, MemoryRegion{reservedMemoryRegions.addr, reservedMemoryRegions.len});
    }

    sanitize(memoryRegions);
    return memoryRegions;
  }

  static constexpr uintptr_t SENTINEL = UINTPTR_MAX;
  struct PhysicalPagesHeader
  {
    uintptr_t prev, next;
    size_t length;
  };

  static PhysicalPagesHeader* getPhysicalPagesHeader(uintptr_t addr)
  {
    return reinterpret_cast<PhysicalPagesHeader*>(MemoryMapping::doFractalMapping(addr, sizeof(PhysicalPagesHeader)));
  }

  static constexpr bool aligned(uintptr_t addr, size_t alignment)
  {
    return addr / alignment * alignment == addr;
  }

  static constexpr uintptr_t alignUp(uintptr_t addr, size_t alignment)
  {
    return (addr + (alignment-1)) / alignment * alignment;
  }

  static uintptr_t m_head;
  void initializePhysical()
  {
    const auto memoryRegions = prepare();

    m_head = memoryRegions.begin()->addr;
    for(auto it = memoryRegions.begin(); it != memoryRegions.end(); ++it)
    {
      rt::logf("begin:%lx, end:%lx\n", it->begin(), it->end());
      auto prev = it != memoryRegions.begin() ? rt::prev(it)->addr : SENTINEL;
      auto next = rt::next(it) != memoryRegions.end() ? rt::next(it)->addr : SENTINEL;

      if(aligned(it->begin(), PAGE_SIZE) || alignUp(it->begin(), PAGE_SIZE) == it->end())
      {
        auto addr   = it->addr;
        auto length = it->length;
        *getPhysicalPagesHeader(addr) = {.prev = prev, .next = next, .length = length };
      }
      else
      {
        auto addr1 = it->begin(), addr2 = alignUp(addr1, PAGE_SIZE), addr3 = it->end();
        *getPhysicalPagesHeader(addr1) = {.prev = prev, .next = addr2, .length = addr2-addr1 };
        *getPhysicalPagesHeader(addr2) = {.prev = addr1, .next = next, .length = addr3-addr2 };
      }
    }
  }

  rt::Optional<Pages> allocPhysicalPages(size_t count)
  {
    PhysicalPagesHeader* header;
    for(auto addr = m_head; addr != SENTINEL; addr = header->next)
    {
      header = getPhysicalPagesHeader(addr);

      size_t length = header->length;
      if(length>=count*PAGE_SIZE)
      {
        uintptr_t prev = header->prev, next = header->next;
        if(length>=count*PAGE_SIZE+sizeof(PhysicalPagesHeader))
        {
          auto  newAddr   = addr + count * PAGE_SIZE;
          auto* newHeader = getPhysicalPagesHeader(newAddr);
          *newHeader = {.prev = prev, .next = next, .length = length - count * PAGE_SIZE};

          if(prev == SENTINEL)
            m_head = newAddr;

          if(prev != SENTINEL)
            getPhysicalPagesHeader(prev)->next = newAddr;

          if(next != SENTINEL)
            getPhysicalPagesHeader(next)->prev = newAddr;
        }
        else
        {
          if(prev == SENTINEL)
            m_head = next;

          if(prev != SENTINEL)
            getPhysicalPagesHeader(prev)->next = next;

          if(next != SENTINEL)
            getPhysicalPagesHeader(next)->prev = prev;
        }

        return Pages{.index = addr / PAGE_SIZE, .count = count};
      }
    }

    return rt::nullOptional;
  }

  void freePhysicalPages(Pages pages)
  {
    auto addr = pages.address();
    auto length = pages.length();

    auto* header = getPhysicalPagesHeader(m_head);
    header->prev = addr;

    auto* newHeader = getPhysicalPagesHeader(addr);
    *newHeader = {.prev = SENTINEL, .next = m_head, .length = length};

    m_head = addr;

  }
}

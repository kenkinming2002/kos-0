#pragma once

#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <optional>

#include <i686/boot/boot.hpp>
#include <boost/intrusive/slist.hpp>

namespace core::memory
{
  using phyaddr_t = uintptr_t;
  using virtaddr_t = uintptr_t;

  constexpr static size_t PAGE_SIZE = 4096;

  struct MemoryRegion : public boost::intrusive::slist_base_hook<boost::intrusive::link_mode<boost::intrusive::normal_link>>
  {
  public:
    static struct IndexPairTag {} index_pair_tag;
    static struct IndexLengthTag {} index_length_tag;
    static struct AddressPairTag {} address_pair_tag;
    static struct AddressLengthTag {} address_length_tag;

  public:
    MemoryRegion() = default;

  public:
    MemoryRegion(size_t beginIndex, size_t endIndex, IndexPairTag) 
      : m_beginIndex(beginIndex), m_endIndex(endIndex) {}
    MemoryRegion(size_t beginIndex, size_t length, IndexLengthTag) 
      : MemoryRegion(beginIndex, beginIndex+length, index_pair_tag) {}

    MemoryRegion(uintptr_t beginAddress, uintptr_t endAddress, AddressPairTag) 
      : m_beginIndex(beginAddress / PAGE_SIZE), m_endIndex(endAddress / PAGE_SIZE) {}
    MemoryRegion(uintptr_t beginAddress, size_t length, AddressLengthTag) 
      : MemoryRegion(beginAddress, beginAddress+length, address_pair_tag) {}

    MemoryRegion(void* begin, void* end) 
      : MemoryRegion(reinterpret_cast<uintptr_t>(begin), reinterpret_cast<uintptr_t>(end), address_pair_tag){}
    MemoryRegion(void* begin, size_t len) 
      : MemoryRegion(reinterpret_cast<uintptr_t>(begin), len, address_length_tag){}

  public:
    MemoryRegion(BootInformation::MemoryMapEntry& memoryMapEntry)
      : MemoryRegion(memoryMapEntry.addr, memoryMapEntry.len, address_length_tag) {}

  public:
    size_t beginIndex() const { return m_beginIndex; }
    size_t endIndex() const { return m_endIndex; }
    size_t count() const { return m_endIndex - m_beginIndex; }
    bool empty() const { return m_beginIndex == m_endIndex; }

    uintptr_t begin() const { return m_beginIndex * PAGE_SIZE; }
    uintptr_t end() const { return m_endIndex * PAGE_SIZE; }

  public:
    MemoryRegion shrink_front(size_t count) { m_beginIndex += count; return MemoryRegion(m_beginIndex-count, m_beginIndex, index_pair_tag); }
    bool tryMergeAfter(MemoryRegion memoryRegion)
    {
      if(m_endIndex==memoryRegion.m_beginIndex)
      {
        m_endIndex = memoryRegion.m_endIndex;
        return true;
      }
      else
        return false;
    }

    bool tryMergeBefore(MemoryRegion memoryRegion)
    {
      if(m_beginIndex==memoryRegion.m_endIndex)
      {
        m_beginIndex = memoryRegion.m_beginIndex;
        return true;
      }
      else
        return false;
    }

  public:
    static std::optional<MemoryRegion> intersection(MemoryRegion lhs, MemoryRegion rhs)
    {
      auto intersection = MemoryRegion(std::max(lhs.m_beginIndex, rhs.m_beginIndex), 
                                       std::min(lhs.m_endIndex,   rhs.m_endIndex), index_pair_tag);

      if(intersection.m_beginIndex>=intersection.m_endIndex)
        return std::nullopt; // No intersection
      else
        return intersection;
    }

    static std::pair<std::optional<MemoryRegion>, std::optional<MemoryRegion>> difference(MemoryRegion lhs, MemoryRegion rhs)
    {
      if(auto intersection = MemoryRegion::intersection(lhs, rhs))
      {
        auto first = MemoryRegion(lhs.m_beginIndex, intersection->m_beginIndex, index_pair_tag);
        auto second = MemoryRegion(intersection->m_endIndex, lhs.m_endIndex, index_pair_tag);
        if(first.empty() && second.empty())
          return {std::nullopt, std::nullopt};
        if(first.empty())
          return {second, std::nullopt};
        if(second.empty())
          return {first, std::nullopt};

        return {first, second};
      }
      else
        return {lhs, std::nullopt};
    }

    friend bool operator>(MemoryRegion lhs, MemoryRegion rhs)
    {
      return lhs.m_beginIndex > rhs.m_beginIndex;
    }

    friend bool operator<(MemoryRegion lhs, MemoryRegion rhs)
    {
      return lhs.m_beginIndex < rhs.m_beginIndex;
    }

  private:
    // NOTE: Maybe use smaller integer type
    size_t m_beginIndex = 0, m_endIndex = 0;
  };


}

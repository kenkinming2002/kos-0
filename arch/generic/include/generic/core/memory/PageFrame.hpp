#pragma once

#include <stddef.h>
#include <stdint.h>

#include <optional>

#include <grub/multiboot2.h>

namespace core::memory
{
  template<size_t PAGE_SIZE=4096u>
  struct PageFrame
  {
  public:
    constexpr static auto SIZE = PAGE_SIZE;

  public:
    char data[PAGE_SIZE];
  } __attribute__((packed));

  /**
   * Pointer to sequential PageFrame[count] at address 
   */
  template<size_t PAGE_SIZE = 4096u>
  struct PageFrameRange
  {
  public:
    constexpr PageFrameRange() = default;
    constexpr PageFrameRange(size_t index, size_t count) : index(index), count(count) {}

    static constexpr PageFrameRange from_index(size_t begin, size_t end)
    {
      return {begin, end - begin};
    }
    static constexpr PageFrameRange from_address(uintptr_t addr, size_t len)
    {
      return {addr / PAGE_SIZE, len / PAGE_SIZE};
    }
    static constexpr PageFrameRange from_multiboot_entry(struct multiboot_mmap_entry& mmap_entry)
    {
      return from_address(mmap_entry.addr, mmap_entry.len);
    }

  public:
    constexpr std::pair<PageFrameRange, std::optional<PageFrameRange>> carve(PageFrameRange& other) const
    {
      if(other > *this || other < *this)
        return std::make_pair(*this, std::nullopt);

      if(other.begin_index() > this->begin_index() &&
         other.end_index()   < this->end_index())
        return std::make_pair(PageFrameRange::from_index(this->begin_index(), other.begin_index()),
                              PageFrameRange::from_index(other.end_index() , this->end_index()));

      if(other.begin_index() > this->begin_index())
        return std::make_pair(PageFrameRange::from_index(this->begin_index(), other.begin_index()), std::nullopt);
      if(other.end_index() < this->end_index())
        return std::make_pair(PageFrameRange::from_index(other.end_index(), this->end_index()), std::nullopt);

      return std::make_pair(*this, std::nullopt); // Should be Unreachable
    }

  public:
    auto begin_index() const { return index; }
    auto end_index() const   { return index + count; }

  public:
    constexpr PageFrame<PAGE_SIZE>* toPageFrames() const 
    { 
      return reinterpret_cast<PageFrame<PAGE_SIZE>*>(index*PAGE_SIZE);
    }

  public:
    friend constexpr bool operator>(const PageFrameRange& lhs, const PageFrameRange& rhs)
    {
      return lhs.begin_index() > rhs.end_index();
    }

    friend constexpr bool operator>=(const PageFrameRange& lhs, const PageFrameRange& rhs)
    {
      return lhs.begin_index() >= rhs.end_index();
    }

    friend constexpr bool operator<(const PageFrameRange& lhs, const PageFrameRange& rhs)
    {
      return lhs.end_index() < rhs.begin_index();
    }

    friend constexpr bool operator<=(const PageFrameRange& lhs, const PageFrameRange& rhs)
    {
      return lhs.end_index() <= rhs.begin_index();
    }

  public:
    // TODO: Consider replacing with smaller integer type
    size_t index = 0;
    size_t count = 0;

  };

  using PhysicalPageFrameRange = PageFrameRange<>;
  using VirtualPageFrameRange  = PageFrameRange<>;
}

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <optional>

#include <generic/grub/multiboot2.h>
#include <boost/intrusive/slist.hpp>

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
  struct PageFrameRange : public boost::intrusive::slist_base_hook<boost::intrusive::link_mode<boost::intrusive::normal_link>>
  {
  public:
    constexpr static auto SIZE = PAGE_SIZE;

  public:
    constexpr PageFrameRange() = default;
    constexpr PageFrameRange(size_t index, size_t count) : index(index), count(count) {}

    // TODO: Tidy up construction function
    static constexpr PageFrameRange from_index(size_t begin, size_t end)
    {
      return {begin, end - begin};
    }
    static constexpr PageFrameRange from_address(uintptr_t addr, size_t len)
    {
      return {(addr + PAGE_SIZE -1) / PAGE_SIZE, len / PAGE_SIZE};
    }
    static constexpr PageFrameRange from_pointers(void* begin, void* end)
    {
      return from_address(reinterpret_cast<uintptr_t>(begin), reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(begin));
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

    constexpr static PageFrameRange fromPageFrames(PageFrame<PAGE_SIZE>* pageFrames, size_t count)
    {
      // Note: pointer to pageFrames must be aligned to PAGE_SIZE as it is
      //       obtained from toPageFrames().
      return PageFrameRange(reinterpret_cast<uintptr_t>(pageFrames) / PAGE_SIZE, count);
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

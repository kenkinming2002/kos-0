#pragma once

#include <librt/NonCopyable.hpp>
#include <librt/Utility.hpp>
#include <librt/Iterator.hpp>

#include <stddef.h>

namespace rt::containers
{
  struct IntrusiveListHook
  {
    IntrusiveListHook* prev = nullptr;
    IntrusiveListHook* next = nullptr;
  };

  class IntrusiveListBase : public NonCopyable
  {
  public:
    class Iterator
    {
    public:
      friend class IntrusiveListBase;

    public:
      using difference_type   = ptrdiff_t;
      using value_type        = const IntrusiveListHook;
      using reference         = value_type&;
      using pointer           = value_type*;

    public:
      constexpr Iterator(pointer hook) : hook(hook) {}

    public:
      constexpr pointer operator->() const { return hook; }
      constexpr reference operator*() const { return *hook; }

    public:
      constexpr Iterator& operator++() { hook = hook->next; return *this; }
      constexpr Iterator operator++(int) { Iterator result = *this; ++(*this); return result; }

    public:
      constexpr Iterator& operator--() { hook = hook->prev; return *this; }
      constexpr Iterator operator--(int) { Iterator result = *this; --(*this); return result; }

    public:
      friend bool operator==(const Iterator& lhs, const Iterator& rhs) { return lhs.hook == rhs.hook; }
      friend bool operator!=(const Iterator& lhs, const Iterator& rhs) { return lhs.hook != rhs.hook; }

    private:
      pointer hook;
    };

  public:
    using const_iterator = Iterator;
    using iterator       = MutableIterator<Iterator>;

  public:
    IntrusiveListBase();
    IntrusiveListBase& operator=(IntrusiveListBase&& other);
    IntrusiveListBase(IntrusiveListBase&& other);

  protected:
    iterator begin() { return iterator(asConst(*this).begin()); }
    iterator end()   { return iterator(asConst(*this).end()); }

    const_iterator begin() const { return const_iterator(m_head.next); }
    const_iterator end()   const { return const_iterator(&m_head); }

  protected:
    iterator insert(const_iterator position, IntrusiveListHook& listIntrusiveListHook);
    iterator remove(const_iterator position, IntrusiveListHook*& listIntrusiveListHook);

  public:
    void splice(const_iterator position, IntrusiveListBase& other);
    void splice(const_iterator position, IntrusiveListBase& other, const_iterator first, const_iterator last);

  public:
    bool empty() const { return begin() == end(); }

  private:
    IntrusiveListHook m_head; // A dummy head
  };

}

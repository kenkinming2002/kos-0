#pragma once

#include <iterator>

#include <librt/NonCopyable.hpp>

#include <stddef.h>

namespace rt::containers
{
  class ListHook
  {
  public:
    constexpr ListHook() = default;

  public:
    template<typename> friend class List;
    ListHook *prev = nullptr, *next = nullptr;
  };

  template<typename ValueType>
  class ListIteratorBase
  {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type   = ptrdiff_t;
    using value_type        = ValueType;
    using pointer           = value_type*;
    using reference         = value_type&;

  public:
    constexpr ListIteratorBase(pointer listHook) : listHook(listHook) {}
    template<typename OtherValueType>
    constexpr ListIteratorBase(ListIteratorBase<OtherValueType> other) : listHook(other.listHook) {}

  public:
    constexpr pointer operator->() const { return listHook; }
    constexpr reference operator*() const { return *listHook; }

  public:
    constexpr ListIteratorBase& operator++() { listHook = listHook->next; return *this; }
    constexpr ListIteratorBase operator++(int) { ListIteratorBase result = *this; ++(*this); return result; }

  public:
    constexpr ListIteratorBase& operator--() { listHook = listHook->prev; return *this; }
    constexpr ListIteratorBase operator--(int) { ListIteratorBase result = *this; --(*this); return result; }

  public:
    pointer listHook;
  };

  template<typename T, typename U>
  bool operator==(const ListIteratorBase<T>& lhs, const ListIteratorBase<U>& rhs) { return lhs.listHook == rhs.listHook; }
  template<typename T, typename U>
  bool operator!=(const ListIteratorBase<T>& lhs, const ListIteratorBase<U>& rhs) { return lhs.listHook != rhs.listHook; }

  class ListBase : NonCopyable
  {
  public:
    using iterator       = ListIteratorBase<ListHook>;
    using const_iterator = ListIteratorBase<const ListHook>;

  public:
    constexpr ListBase() { m_head.prev = &m_head; m_head.next = &m_head; }

  protected:
    constexpr ~ListBase() = default;

  protected:
    iterator begin() { return iterator(m_head.next); }
    iterator end()   { return iterator(&m_head); }

    const_iterator begin() const { return const_iterator(const_cast<ListHook*>(m_head.next)); }
    const_iterator end()   const { return const_iterator(const_cast<ListHook*>(&m_head)); }

  public:
    bool empty() const { return begin() == end(); }

  protected:
    iterator insert(const_iterator position, ListHook& listHook);
    iterator remove(const_iterator position, ListHook*& listHook);

  public:
    void splice(const_iterator position, ListBase& other, const_iterator first, const_iterator last);

  private:
    ListHook m_head; // A dummy head
  };

}

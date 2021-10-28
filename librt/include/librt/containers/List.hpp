#pragma once

#include <librt/containers/IntrusiveList.hpp>

namespace rt::containers
{
  template<typename T>
  struct ListNode : public IntrusiveListHook
  {
    T value;
  };

  template<typename T>
  class List : public IntrusiveList<ListNode<T>>
  {
  public:
    using Node = ListNode<T>;
    using Base = IntrusiveList<Node>;

  public:
    class Iterator : public IntrusiveList<Node>::Iterator
    {
    public:
      using Base = typename IntrusiveList<Node>::Iterator;

    public:
      using value_type        = const T;
      using pointer           = value_type*;
      using reference         = value_type&;

    public:
      constexpr Iterator(const Base& base) : Base(base) {}
      using Base::Base;

    public:
      constexpr pointer operator->()  const { return &Base::operator*().value; }
      constexpr reference operator*() const { return Base::operator*().value; }

    public:
      using Base::operator++;
      using Base::operator--;
    };

  public:
    using iterator        = MutableIterator<Iterator>;
    using const_iterator  = Iterator;
    using value_type      = T;
    using pointer         = value_type*;
    using reference       = value_type&;
    using const_pointer   = const value_type*;
    using const_reference = const value_type&;

  public:
    constexpr List() = default;
    constexpr ~List() { clear(); }

  public:
    List& operator=(List&& other) = default;
    List(List&& other) = default;

  public:
    List& operator=(const List& other)
    {
      clear();
      for(const auto& value : other)
        insert(end(), value);

      return *this;
    }
    List(const List& other) : List() { *this = other; }


  public:
    const_iterator begin() const { return Base::begin(); }
    const_iterator end()   const { return Base::end(); }

    iterator begin() { return asConst(*this).begin(); }
    iterator end()   { return asConst(*this).end(); }

  private:
    static constexpr auto disposer = [](Node* node){ delete node; };

  public:
    template<typename... Args>
    iterator emplace(const_iterator position, Args&&... args)
    {
      auto* node = new Node{.value = Value(forwart<Args>(args)...)};
      return Base::insert(position, *node);
    }

    iterator insert(const_iterator position, value_type&& value)
    {
      auto* node = new Node{.value = move(value)};
      return Base::insert(position, *node);
    }

    iterator insert(const_iterator position, const value_type& value)
    {
      auto* node = new Node{.value = value};
      return Base::insert(position, *node);
    }

    iterator remove(const_iterator position) { return Base::remove_and_dispose(position, disposer); }
    void clear() { Base::clear_and_dispose(disposer); }

  public:
    using Base::empty;

  public:
    value_type& front() { ASSERT(!empty()); return *begin(); }
    value_type& back()  { ASSERT(!empty()); return *prev(end()); }

    const value_type& front() const { ASSERT(!empty()); return *begin(); }
    const value_type& back()  const { ASSERT(!empty()); return *prev(end()); }

  public:
    void popFront() { ASSERT(!empty()); remove(begin()); }
    void popBack()  { ASSERT(!empty()); remove(prev(end())); }

  public:
    void splice(const_iterator position, List& other, const_iterator first, const_iterator last)
    {
      Base::splice(position, other, first, last);
    }
  };
}


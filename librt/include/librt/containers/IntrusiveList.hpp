#pragma once

#include <librt/containers/IntrusiveListBase.hpp>

namespace rt::containers
{
  template<typename T>
  class IntrusiveList : public IntrusiveListBase
  {
  public:
    using Base = IntrusiveListBase;

  public:
    static_assert(std::is_base_of_v<IntrusiveListHook, T>);

  public:
    class Iterator : public IntrusiveListBase::Iterator
    {
    public:
      using Base = IntrusiveListBase::Iterator;

    public:
      using value_type        = const T;
      using pointer           = value_type*;
      using reference         = value_type&;

    public:
      constexpr Iterator(Base base) : Base(base) {}
      using Base::Base;

    public:
      constexpr pointer operator->() const { return static_cast<pointer>(Base::operator->()); }
      constexpr reference operator*() const { return static_cast<reference>(Base::operator*()); }

    public:
      constexpr Iterator& operator++() { Base::operator++(); return *this;}
      constexpr Iterator operator++(int) { return static_cast<Iterator>(Base::operator++(0)); }

    public:
      constexpr Iterator& operator--() { Base::operator--(); return *this;}
      constexpr Iterator operator--(int) { return Base::operator--(0); }
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
    using Base::Base;

  public:
    iterator begin() { return Base::begin(); }
    iterator end()   { return Base::end(); }

    const_iterator begin() const { return Base::begin(); }
    const_iterator end()   const { return Base::end(); }

  public:
    iterator insert(const_iterator position, reference value)        { return Base::insert(position, value); }

    template<typename Disposer>
    iterator remove_and_dispose(const_iterator position, Disposer disposer)
    {
      IntrusiveListHook* hook;
      auto result = Base::remove(position, hook);
      disposer(static_cast<pointer>(hook));
      return result;
    }

    template<typename Disposer>
    void clear_and_dispose(Disposer disposer)
    {
      while(!empty())
        remove_and_dispose(begin(), disposer);
    }

  public:
    void splice(const_iterator position, IntrusiveList& other, const_iterator first, const_iterator last)
    {
      Base::splice(position, other, first, last);
    }
  };

  struct Test : public IntrusiveListHook {};
  template class IntrusiveList<Test>;
}

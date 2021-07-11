#pragma once

#include <librt/containers/ListBase.hpp>

namespace rt::containers
{
  template<typename T>
  class ListIterator : public ListIteratorBase<std::conditional_t<std::is_const_v<T>, const ListHook, ListHook>>
  {
  public:
    using Base = ListIteratorBase<std::conditional_t<std::is_const_v<T>, const ListHook, ListHook>>;

  public:
    using value_type        = T;
    using pointer           = value_type*;
    using reference         = value_type&;

  public:
    constexpr ListIterator(const Base& base) : Base(base) {}
    template<typename U>
    constexpr ListIterator(ListIterator<U> other) : ListIterator(static_cast<typename ListIterator<U>::Base>(other)) {}

  public:
    constexpr pointer operator->() const { return static_cast<pointer>(Base::operator->()); }
    constexpr reference operator*() const { return static_cast<reference>(Base::operator*()); }

  public:
    constexpr ListIterator& operator++() { Base::operator++(); return *this;}
    constexpr ListIterator operator++(int) { return Base::operator++(0); }

  public:
    constexpr ListIterator& operator--() { Base::operator--(); return *this;}
    constexpr ListIterator operator--(int) { return Base::operator--(0); }
  };

  template<typename T>
  class List : public ListBase
  {
  public:
    static_assert(std::is_base_of_v<ListHook, T>);

  private:
    using Base = ListBase;

  public:
    using Base::Base;

  public:
    using iterator       = ListIterator<T>;
    using const_iterator = ListIterator<const T>;
    using value_type     = typename iterator::value_type;
    using pointer        = typename iterator::pointer;
    using reference      = typename iterator::reference;

  public:
    iterator begin() { return Base::begin(); }
    iterator end()   { return Base::end(); }

    const_iterator begin() const { return Base::begin(); }
    const_iterator end()   const { return Base::end(); }

  public:
    iterator insert(const_iterator position, reference value)        { return Base::insert(position, value); }

    /* We are borrowing terminology from boost intrusive */
    template<typename Disposer>
    iterator remove_and_dispose(const_iterator position, Disposer disposer)
    {
      ListHook* listHook;
      auto result = Base::remove(position, listHook);
      disposer(static_cast<pointer>(listHook));
      return result;
    }

    template<typename Disposer>
    void clear_and_dispose(Disposer disposer)
    {
      while(!empty())
        remove_and_dispose(begin(), disposer);
    }

  public:
    void splice(const_iterator position, List& other, const_iterator first, const_iterator last)
    {
      Base::splice(position, other, first, last);
    }
  };
}

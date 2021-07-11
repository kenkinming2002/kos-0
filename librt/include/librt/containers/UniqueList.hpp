#pragma once

#include <librt/containers/List.hpp>

#include <librt/UniquePtr.hpp>

namespace rt::containers
{
  template<typename T>
  class UniqueListIterator : public ListIterator<T>
  {
  public:
    using Base = ListIterator<T>;

  public:
    using typename Base::value_type;
    using typename Base::pointer;
    using typename Base::reference;
    using smart_pointer = rt::UniquePtr<value_type>;

  public:
    template<typename U>
    constexpr UniqueListIterator(UniqueListIterator<U> other) : UniqueListIterator(static_cast<typename UniqueListIterator<U>::Base>(other)) {}

  public:
    using Base::Base;

    using Base::operator->;
    using Base::operator*;

    using Base::operator++;
    using Base::operator--;
  };

  template<typename T>
  class UniqueList : public List<T>
  {
  public:
    static_assert(std::is_base_of_v<ListHook, T>);

  private:
    using Base = List<T>;

  public:
    using Base::Base;
    ~UniqueList() { clear(); }

  public:
    using iterator       = UniqueListIterator<T>;
    using const_iterator = UniqueListIterator<const T>;

  public:
    using value_type     = typename iterator::value_type;
    using pointer        = typename iterator::pointer;
    using reference      = typename iterator::reference;
    using smart_pointer  = typename iterator::smart_pointer;

  public:
    iterator begin() { return Base::begin(); }
    iterator end()   { return Base::end(); }

    const_iterator begin() const { return Base::begin(); }
    const_iterator end()   const { return Base::end(); }

  private:
    /* We are borrowing terminology from boost intrusive */
    struct UniqueDisposer { void operator()(typename Base::pointer ptr) { (void)UniquePtr<T>(ptr); } };

  public:
    iterator insert(const_iterator position, smart_pointer ptr)        { ASSERT(ptr); return Base::insert(position, *ptr.release()); }
    iterator remove(const_iterator position) { return Base::remove_and_dispose(position, UniqueDisposer{}); }
    void clear()                             { Base::clear_and_dispose(UniqueDisposer{}); }

  public:
    void splice(const_iterator position, UniqueList& other, const_iterator first, const_iterator last)
    {
      Base::splice(position, other, first, last);
    }
  };


  struct UniqueTest : public ListHook {};
  template class UniqueList<UniqueTest>;
}


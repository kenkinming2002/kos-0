#pragma once

#include <librt/containers/List.hpp>

#include <librt/SharedPtr.hpp>

namespace rt::containers
{
  template<typename T>
  class SharedListIterator : public ListIterator<T>
  {
  public:
    using Base = ListIterator<T>;

  public:
    using typename Base::value_type;
    using typename Base::pointer;
    using typename Base::reference;
    using smart_pointer = rt::SharedPtr<value_type>;

  public:
    template<typename U>
    constexpr SharedListIterator(SharedListIterator<U> other) : SharedListIterator(static_cast<typename SharedListIterator<U>::Base>(other)) {}

  public:
    using Base::Base;

    using Base::operator->;
    using Base::operator*;

    using Base::operator++;
    using Base::operator--;

  public:
    constexpr auto get() const  { return smart_pointer(operator->()); }
  };

  template<typename T>
  class SharedList : public List<T>
  {
  public:
    static_assert(std::is_base_of_v<ListHook, T>);
    static_assert(std::is_base_of_v<SharedPtrHook, T>);

  private:
    using Base = List<T>;

  public:
    using Base::Base;
    ~SharedList() { clear(); }

  public:
    using iterator       = SharedListIterator<T>;
    using const_iterator = SharedListIterator<const T>;

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
    struct SharedDisposer { void operator()(typename Base::pointer ptr) { SharedPtr<T>().adopt(ptr); } };

  public:
    iterator insert(const_iterator position, smart_pointer ptr)        { ASSERT(ptr); return Base::insert(position, ptr.release()); }
    iterator remove(const_iterator position) { return Base::remove_and_dispose(position, SharedDisposer{}); }
    void clear()                             { Base::clear_and_dispose(SharedDisposer{}); }

  public:
    void splice(const_iterator position, SharedList& other, const_iterator first, const_iterator last)
    {
      Base::splice(position, other, first, last);
    }
  };


  struct SharedTest : public ListHook, SharedPtrHook {};
  template class SharedList<SharedTest>;
}

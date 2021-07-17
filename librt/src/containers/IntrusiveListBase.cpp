#include <librt/containers/IntrusiveListBase.hpp>

#include <librt/Assert.hpp>
#include <librt/Log.hpp>

namespace rt::containers
{
  namespace
  {
    void link(IntrusiveListHook* lhs, IntrusiveListHook* rhs)
    {
      lhs->next = rhs;
      rhs->prev = lhs;
    }
  }

  IntrusiveListBase::IntrusiveListBase() { link(&m_head, &m_head); }

  IntrusiveListBase& IntrusiveListBase::operator=(IntrusiveListBase&& other)
  {
    IntrusiveListBase tmp;
    tmp.splice(tmp.end(), *this);
    this->splice(this->end(), other);
    other.splice(other.end(), tmp);
    return *this;
  }

  IntrusiveListBase::IntrusiveListBase(IntrusiveListBase&& other) : IntrusiveListBase() { *this = move(other); }

  IntrusiveListBase::iterator IntrusiveListBase::insert(const_iterator position, IntrusiveListHook& listHook)
  {
    auto* prev = const_cast<IntrusiveListHook*>(position.hook->prev);
    auto* next = const_cast<IntrusiveListHook*>(position.hook);
    link(prev, &listHook);
    link(&listHook, next);
    return iterator(&listHook);
  }

  IntrusiveListBase::iterator IntrusiveListBase::remove(const_iterator position, IntrusiveListHook*& listHook)
  {
    listHook = const_cast<IntrusiveListHook*>(position.hook);
    auto* prev = listHook->prev;
    auto* next = listHook->next;
    link(prev, next);
    return iterator(next);
  }

  void IntrusiveListBase::splice(const_iterator position, IntrusiveListBase& other)
  {
    splice(position, other, other.begin(), other.end());
  }

  void IntrusiveListBase::splice(const_iterator position, IntrusiveListBase& other, const_iterator first, const_iterator last)
  {
    if(first == last)
      return; // Splicing nothing is a no-op

    // Unlink target from other list
    auto* front = const_cast<IntrusiveListHook*>(first.hook);
    auto* back  = const_cast<IntrusiveListHook*>(last.hook->prev);
    link(front->prev, back->next);

    // Link target
    auto* prev = const_cast<IntrusiveListHook*>(position.hook->prev);
    auto* next = const_cast<IntrusiveListHook*>(position.hook);
    link(prev, front);
    link(back, next);
  }
}

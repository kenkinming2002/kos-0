#include <librt/containers/List.hpp>

namespace rt::containers
{
  namespace
  {
    void link(ListHook* lhs, ListHook* rhs)
    {
      lhs->next = rhs;
      rhs->prev = lhs;
    }
  }
  ListBase::iterator ListBase::insert(const_iterator position, ListHook& listHook)
  {
    auto* prev = const_cast<ListHook*>(position.listHook->prev);
    auto* next = const_cast<ListHook*>(position.listHook);
    link(prev, &listHook);
    link(&listHook, next);
    return iterator(&listHook);
  }

  ListBase::iterator ListBase::remove(const_iterator position, ListHook*& listHook)
  {
    listHook = const_cast<ListHook*>(position.listHook);
    auto* prev = listHook->prev;
    auto* next = listHook->next;
    link(prev, next);
    return iterator(next);
  }

  void ListBase::splice(const_iterator position, ListBase& other, const_iterator first, const_iterator last)
  {
    // Unlink target from other list
    auto* front = const_cast<ListHook*>(first.listHook);
    auto* back= const_cast<ListHook*>(last.listHook->prev);
    link(front->prev, back->next);

    // Link target
    auto* prev = const_cast<ListHook*>(position.listHook->prev);
    auto* next = const_cast<ListHook*>(position.listHook);
    link(prev, front);
    link(back, next);
  }
}

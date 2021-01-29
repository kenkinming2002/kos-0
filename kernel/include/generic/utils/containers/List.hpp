#pragma once

#include <stddef.h>
#include <iterator>
#include <type_traits>

namespace utils::containers
{
  template<typename T>
  class List
  {
  public:
    constexpr List() { m_tail.prev = m_tail.next = &m_tail; }

  public:
    List(const List&) = delete;
    List(List&&) = delete;
    List& operator=(const List&) = delete;
    List& operator=(List&&) = delete;

  public:
    struct NodeBase
    {
      mutable NodeBase *prev, *next;
    };

    struct Node : public NodeBase
    {
      T value;
    };

    template<typename U>
    struct iterator_base
    {
    public:
      using iterator_category = std::bidirectional_iterator_tag;
      using value_type        = U;
      using difference_type   = ptrdiff_t;
      using pointer           = U*;
      using reference         = U&;

    public:
      iterator_base(NodeBase* nodeBase) : nodeBase(nodeBase) {}
      template<typename V>
      iterator_base(iterator_base<V> other) : nodeBase(other.nodeBase) {}

    public:
      pointer operator->() const { return &static_cast<Node*>(nodeBase)->value; }
      reference operator*() const { return static_cast<Node*>(nodeBase)->value; }

    public:
      template<typename V>
      bool operator==(const iterator_base<V>& other) const { return nodeBase == other.nodeBase; }
      template<typename V>
      bool operator!=(const iterator_base<V>& other) const { return nodeBase != other.nodeBase; }

    public:
      iterator_base& operator++() { nodeBase = nodeBase->next; return *this; }
      iterator_base operator++(int) { iterator_base result = *this; ++(*this); return result; }

    public:
      iterator_base& operator--() { nodeBase = nodeBase->prev; return *this; }
      iterator_base operator--(int) { iterator_base result = *this; --(*this); return result; }

    public:
      NodeBase* nodeBase;
    };

  public:
    using value_type     = T;
    using iterator       = iterator_base<T>;
    using const_iterator = iterator_base<const T>;

  public:
    iterator begin() { return iterator(m_tail.next); }
    iterator end()   { return iterator(&m_tail); }

    const_iterator begin() const { return const_iterator(m_tail.next); }
    const_iterator end()   const { return const_iterator(&m_tail); }

  public:
    iterator insert(const_iterator position, const value_type& value)
    {
      auto* nodeBase = static_cast<NodeBase*>(new Node{{nullptr, nullptr}, value});

      nodeBase->next = position.nodeBase;
      nodeBase->prev = position.nodeBase->prev;

      position.nodeBase->prev->next = nodeBase;
      position.nodeBase->prev       = nodeBase;

      return iterator(nodeBase);
    }

    iterator insert(const_iterator position, value_type&& value)
    {
      auto* nodeBase = static_cast<NodeBase*>(new Node{{nullptr, nullptr}, std::move(value)});

      nodeBase->next = position.nodeBase;
      nodeBase->prev = position.nodeBase->prev;

      position.nodeBase->prev->next = nodeBase;
      position.nodeBase->prev       = nodeBase;

      return iterator(nodeBase);
    }

    iterator erase(const_iterator position)
    {
      auto result = position.nodeBase->next;

      position.nodeBase->prev->next = position.nodeBase->next;
      position.nodeBase->next->prev = position.nodeBase->prev;

      delete static_cast<Node*>(position.nodeBase);
      return result;
    }

  public:
    bool empty() const { return begin() == end(); }

  private:
    mutable NodeBase m_tail; // Dummy element
  };
}

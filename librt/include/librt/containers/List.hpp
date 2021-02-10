#pragma once

#include <iterator>
#include <librt/Utility.hpp>

#include <stddef.h>

namespace rt::containers
{
  template<typename T>
  class List
  {
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
    struct IteratorBase
    {
    public:
      using iterator_category = std::bidirectional_iterator_tag;
      using value_type        = U;
      using difference_type   = ptrdiff_t;
      using pointer           = U*;
      using reference         = U&;

    public:
      IteratorBase(NodeBase* nodeBase) : nodeBase(nodeBase) {}
      template<typename V>
      IteratorBase(IteratorBase<V> other) : nodeBase(other.nodeBase) {}

    public:
      pointer operator->() const { return &static_cast<Node*>(nodeBase)->value; }
      reference operator*() const { return static_cast<Node*>(nodeBase)->value; }

    public:
      template<typename V>
      bool operator==(const IteratorBase<V>& other) const { return nodeBase == other.nodeBase; }
      template<typename V>
      bool operator!=(const IteratorBase<V>& other) const { return nodeBase != other.nodeBase; }

    public:
      IteratorBase& operator++() { nodeBase = nodeBase->next; return *this; }
      IteratorBase operator++(int) { IteratorBase result = *this; ++(*this); return result; }

    public:
      IteratorBase& operator--() { nodeBase = nodeBase->prev; return *this; }
      IteratorBase operator--(int) { IteratorBase result = *this; --(*this); return result; }

    public:
      NodeBase* nodeBase;
    };

  public:
    using value_type     = T;
    using iterator       = IteratorBase<T>;
    using const_iterator = IteratorBase<const T>;

  public:
    constexpr List() { m_tail.prev = m_tail.next = &m_tail; }

  public:
    List(const List&) = delete;
    List(List&&) = delete;
    List& operator=(const List&) = delete;
    List& operator=(List&&) = delete;


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
      auto* nodeBase = static_cast<NodeBase*>(new Node{{nullptr, nullptr}, move(value)});

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

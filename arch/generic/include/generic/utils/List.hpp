#pragma once

#include <utils/Pointer.hpp>
#include <utils/Utilities.hpp>

namespace utils
{
  template<typename T, typename Allocator>
  class ForwardList
  {
  public:
    ForwardList(const ForwardList& other) = delete;
    ForwardList(ForwardList&& other) : m_head(utils::move(other.m_head)), m_allocator(utils::move(other.m_allocator)) {}
    template<typename OtherAllocator>
    ForwardList(ForwardList<T, OtherAllocator>&& other) : m_head(utils::move(other.m_head)), m_allocator(utils::move(other.m_allocator)) {}

    ForwardList& operator=(const ForwardList& other) = delete;
    ForwardList& operator=(ForwardList&& other)
    {
      m_head = utils::move(other.m_head);
      m_allocator = utils::move(other.m_allocator);
    }
    template<typename OtherAllocator>
    ForwardList& operator=(ForwardList<T, OtherAllocator>&& other) 
    {
      m_head = utils::move(other.m_head);
      m_allocator = utils::move(other.m_allocator);
    }

  public:
    ForwardList(const Allocator& allocator = Allocator()) : m_head{nullptr}, m_allocator() {}

  public:
    struct NodeBase;
    struct Node;
    template<typename NodeBase, typename Node>
    class iterator_base;

    using const_iterator = iterator_base<const NodeBase, const Node>;
    using iterator       = iterator_base<NodeBase, Node>;

  public:
    void insert(iterator it, T data)
    {
      Node* node = m_allocator.allocate(1);
      m_allocator.construct(node, it.m_node->next, data);
      it.m_node->next = node;
    }

    void push_front(T data) 
    {
      Node* node = m_allocator.allocate(1);
      m_allocator.construct(node, m_head.next, data);
      m_head.next = node;
    }

    void erase_after(iterator it)
    {
      NodeBase* next = it.m_node->next->next.get();
      m_allocator.deallocate(static_cast<Node*>(it.m_node->next.get()), 1);
      it.m_node->next = next;
    }


  public:
    iterator before_begin();
    const_iterator cbefore_begin() const;

    iterator begin();
    const_iterator cbegin() const;

    iterator end();
    const_iterator cend() const;

  private:
    NodeBase m_head;
    [[no_unique_address]] typename Allocator::template rebind<Node> m_allocator;
  };

  template<typename T, typename Allocator>
  struct ForwardList<T, Allocator>::NodeBase
  {
    UniquePointer<NodeBase> next;
  };

  template<typename T, typename Allocator>
  struct ForwardList<T, Allocator>::Node : public NodeBase
  {
    T data;
  };

  template<typename T, typename Allocator>
  template<typename NodeBase, typename Node>
  class ForwardList<T, Allocator>::iterator_base
  {
  public:
    using node_pointer = NodeBase*;
    using node_reference = NodeBase&;

  public:
    iterator_base(node_pointer node) : m_node(node) {}

  public:
    iterator& operator++() { m_node = m_node->next.get(); return *this; }
    iterator operator++(int) { iterator it = *this; m_node = m_node->next.get(); return it; }

    operator bool() const { return m_node; }

    bool operator==(iterator_base other) const { return m_node == other->m_node; }
    bool operator!=(iterator_base other) const { return m_node != other.m_node; }

    auto& operator*() { return static_cast<Node*>(m_node)->data; }
    auto operator->() { return &static_cast<Node*>(m_node)->data; }

  public:
    node_pointer m_node;
  };

  template<typename T, typename Allocator>
  typename ForwardList<T, Allocator>::iterator ForwardList<T, Allocator>::before_begin() { return iterator(&m_head); }
  template<typename T, typename Allocator>
  typename ForwardList<T, Allocator>::const_iterator ForwardList<T, Allocator>::cbefore_begin() const { return const_iterator(&m_head); }

  template<typename T, typename Allocator>
  typename ForwardList<T, Allocator>::iterator ForwardList<T, Allocator>::begin() { return iterator(m_head.next.get()); }
  template<typename T, typename Allocator>
  typename ForwardList<T, Allocator>::const_iterator ForwardList<T, Allocator>::cbegin() const { return const_iterator(m_head.next.get()); }

  template<typename T, typename Allocator>
  typename ForwardList<T, Allocator>::iterator ForwardList<T, Allocator>::end() { return iterator(nullptr); }
  template<typename T, typename Allocator>
  typename ForwardList<T, Allocator>::const_iterator ForwardList<T, Allocator>::cend() const { return const_iterator(nullptr); }
}

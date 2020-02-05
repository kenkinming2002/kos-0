#pragma once

#include <utils/Utilities.hpp>

#include <stddef.h>

#include <memory>
#include <iterator>

#include <io/Print.hpp>

namespace containers
{
  /** Base class for ForwardList
   *  
   *  Can only perform transversal, element insertion and deletion, but do not
   *  manage any allocation.
   */
  template<typename T>
  class ForwardListBase
  {
  public:
    ForwardListBase() = default;

  public:
    // ForwardListBase does not manage any memory, and thus can not perform any
    // form of deep copying behavior
    ForwardListBase(const ForwardListBase& other) = delete;
    ForwardListBase(ForwardListBase&& other) : m_head(std::exchange(other.m_head, NodeBase())) {}

  // node
  public:
    struct NodeBase
    {
    public:
      NodeBase* next = nullptr;
    };

    struct Node : public NodeBase
    {
      explicit Node(const T& data) : data(data) {}
      explicit Node(T&& data) : data(std::move(data)) {}

      T data;
    };

  // iterator
  public:
    template<typename Value, typename NodeBase, typename Node>
    class iterator_base
    {
    public:
      using difference_type   = ptrdiff_t;
      using value_type        = Value;
      using pointer           = Value*;
      using reference         = Value&;
      using iterator_category = std::forward_iterator_tag;

    public:
      iterator_base() = default;
      iterator_base(NodeBase* node) : m_node(node) {}

    public:
      auto& operator++() 
      { 
        m_node = m_node->next;
        return *this; 
      }
      auto operator++(int) 
      { 
        auto it = *this;
        m_node = m_node->next;
        return it;
      }

      bool operator==(iterator_base other) const { return m_node == other->m_node; }
      bool operator!=(iterator_base other) const { return m_node != other.m_node; }

      // NOTE: the behavior is undefined if you try to dereference a
      //       before_begin iterator
      auto& operator*() { return static_cast<Node*>(m_node)->data; }
      auto operator->() { return &static_cast<Node*>(m_node)->data; }

    public:
      constexpr auto node() { return m_node; }

    private:
      NodeBase* m_node = nullptr;
    };
    using iterator       = iterator_base<T, NodeBase, Node>;
    using const_iterator = iterator_base<T, const NodeBase, const Node>;

  public:
    iterator before_begin() { return iterator(&m_head); }
    const_iterator before_begin() const { return iterator(&m_head); }
    const_iterator cbefore_begin() const { return iterator(&m_head); }

    iterator begin() { return iterator(m_head.next); }
    const_iterator begin() const { return iterator(m_head.next); }
    const_iterator cbegin() const { return iterator(m_head.next); }

    iterator end() { return iterator(); }
    const_iterator end() const { return iterator(); }
    const_iterator cend() const { return iterator(); }

  public:
    iterator insert_after(iterator it, Node* node)
    {
      node->next = it.node()->next;
      it.node()->next = node;
      return iterator(node);
    }

    void push_front(Node* node)
    {
      insert_after(before_begin(), node);
    }

    Node* erase_after(iterator it)
    {
      Node* erasedNode = static_cast<Node*>(it.node()->next);
      it.node()->next = it.node()->next->next;
      return erasedNode;
    }

    Node* pop_front()
    {
      return erase_after(before_begin());
    }

    bool empty() const
    {
      return m_head.next == nullptr;
    }

  protected:
    NodeBase m_head;
  };

  /** Actual ForwardList class
   *  
   *  Add allocation functionality to ForwardList
   */
  template<typename T, typename Allocator>
  class ForwardList : public ForwardListBase<T>
  {
  public:
    using typename ForwardListBase<T>::Node;
    using typename ForwardListBase<T>::NodeBase;

    using typename ForwardListBase<T>::iterator;
    using typename ForwardListBase<T>::const_iterator;

  public:
    using allocator_traits = typename std::allocator_traits<Allocator>::template rebind_traits<Node>;
    using allocator_type   = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;

  public:
    ForwardList() = default;
    ForwardList(const Allocator& allocator) : m_allocator(allocator) {}

  public:
    ~ForwardList()
    {
      auto current = static_cast<Node*>(ForwardListBase<T>::m_head.next);
      while(current)
      {
        auto next = static_cast<Node*>(current->next);
        m_allocator.deallocate(current, 1);
        current = next;
      }
    }

  public:
    ForwardList(const ForwardList& other) : m_allocator(allocator_traits::select_on_container_copy_construction(other.m_allocator))
    {
      auto it = this->before_begin();
      for(const auto& elem : other)
        it = this->insert_after(it, elem);
    }
    template<typename OtherAllocator>
    ForwardList(const ForwardList<T, OtherAllocator>& other) : m_allocator()
    {
      auto it = this->before_begin();
      for(const auto& elem : other)
        it = this->insert_after(it, elem);
    }
    ForwardList(ForwardList&& other) : ForwardListBase<T>(std::move(other)), m_allocator(std::move(other.allocator)) {}

  public:
    iterator insert_after(iterator it, T data)
    {
      auto node = allocator_traits::allocate(m_allocator, 1);
      allocator_traits::construct(m_allocator, node,  data);

      return this->ForwardListBase<T>::insert_after(it, node);
    }

    void push_front(T data) 
    {
      Node* node = allocator_traits::allocate(m_allocator, 1);
      allocator_traits::construct(m_allocator, node, data);
      return this->ForwardListBase<T>::push_front(node);
    }

    void erase_after(iterator it)
    {
      auto node = this->ForwardListBase<T>::erase_after(it);
      m_allocator.deallocate(node, 1);
    }

    void pop_front()
    {
      auto node = this->ForwardListBase<T>::pop_front();
      m_allocator.deallocate(node, 1);
    }

  public:
    ForwardListBase<T>& base() { return *this; }
    const ForwardListBase<T>& base() const { return *this; }

  private:
    allocator_type m_allocator;
  };
}

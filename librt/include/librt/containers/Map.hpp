#pragma once

#include <librt/UniquePtr.hpp>
#include <librt/Pair.hpp>
#include <librt/Utility.hpp>
#include <librt/Assert.hpp>

#include <compare>

namespace rt::containers
{
  /* FIXME: Our map implementation is really horrible, in that our code is duplicated
   * with each instantiation, and tha is causing massive code bloat, we need to
   * do something about it, such as some form of type erasure.
   *
   * That is why tenplate is not always the best solution
   *
   * TODO: We need to allow incorporation of smart ptr directly into
   * containers to prevent double allocation */
  template<typename Key, typename T>
  class Map
  {
  public:
    struct Node
    {
    public:
      using mutable_value_type = Pair<Key, T>;
      using value_type         = Pair<const Key, T>;

      Node* parent = nullptr;
      UniquePtr<Node> left = nullptr, right = nullptr;

      value_type value;

    public:
      bool isRoot()  const { return !parent; }
      bool isLeave() const { return !left && !right; }
      bool isLeftChild() const { ASSERT(parent); return parent->left.get() == this; }
      bool isRightChild() const { ASSERT(parent); return parent->right.get() == this; }
      const Node* leftist()  const { const Node* current = this; while(current->left)  current = current->left.get();  return current; }
      const Node* rightist() const { const Node* current = this; while(current->right) current = current->right.get(); return current; }
      Node* leftist()  { return const_cast<Node*>(asConst(*this).leftist()); }
      Node* rightist() { return const_cast<Node*>(asConst(*this).rightist()); }
      const UniquePtr<Node>& owned() const { ASSERT(isLeftChild() || isRightChild()); return isLeftChild() ? parent->left : parent->right; }
      UniquePtr<Node>& owned() { return const_cast<UniquePtr<Node>&>(asConst(*this).owned()); }

      Node* insert(mutable_value_type value)
      {
        if(value.first<this->value.first)
        {
          if(left)
          {
            return left->insert(move(value));
          }
          else
          {
            left = makeUnique<Node>(this, nullptr, nullptr, move(value));
            return left.get();
          }
        }
        else if(this->value.first<value.first)
        {
          if(right)
          {
            return right->insert(move(value));
          }
          else
          {
            right = makeUnique<Node>(this, nullptr, nullptr, move(value));
            return right.get();
          }
        }
        else
          return nullptr;
      }

      rt::UniquePtr<Node> erase(rt::UniquePtr<Node>& root)
      {
        auto& node = isRoot() ? root : owned();
        ASSERT(node.get() == this);

        if(isLeave())
          return move(node);

        if(left && !left->right)
        {
          left->parent = parent;
          left->right = move(right);
          return exchange(node, move(left));
        }

        if(right && !right->left)
        {
          right->parent = parent;
          right->left = move(left);
          return exchange(node, move(right));
        }

        auto replacement = left ? left->rightist()->erase(root) : right->leftist()->erase(root);
        ASSERT(!replacement->left && !replacement->right);

        replacement->parent = parent;
        replacement->left   = move(left);
        replacement->right  = move(right);
        replacement->left->parent  = replacement.get();
        replacement->right->parent = replacement.get();

        return exchange(node, move(replacement));
      }

      template<typename K>
      const Node* find(const K& key) const
      {
        if(key<this->value.first)
          return left ? left->find(key) : nullptr;
        else if(this->value.first<key)
          return right ? right->find(key) : nullptr;
        else
          return this;
      }

      template<typename K>
      Node* find(const K& key) { return const_cast<Node*>(asConst(*this).find(key)); }
    };

    template<typename NodeType, typename ValueType>
    struct IteratorBase
    {
    public:
      using value_type        = ValueType;
      using difference_type   = ptrdiff_t;
      using pointer           = ValueType*;
      using reference         = ValueType&;

    public:
      constexpr IteratorBase() : node(nullptr) {}
      constexpr IteratorBase(NodeType* node) : node(node) {}
      template<typename U, typename V>
      constexpr IteratorBase(const IteratorBase<U, V>& other) : node(other.node) {}

    public:
      constexpr pointer operator->() const { return &node->value; }
      constexpr reference operator*() const { return node->value; }

    public:
      constexpr friend bool operator==(const IteratorBase&, const IteratorBase&) = default;
      constexpr friend bool operator!=(const IteratorBase&, const IteratorBase&) = default;

    public:
      IteratorBase& operator++()
      {
        if(node->right)
        {
          node = node->right->leftist();
          return *this;
        }

        while(node->parent)
        {
          if(node->isLeftChild())
          {
            node = node->parent;
            return *this;
          }
          else
            node = node->parent;
        }

        node = nullptr;
        return *this;
      }
      IteratorBase operator++(int) { IteratorBase result = *this; ++(*this); return result; }

    public:
      IteratorBase& operator--()
      {
        if(node->left)
        {
          node = node->left->rightist();
          return *this;
        }

        while(node->parent)
        {
          if(node->isRightChild())
          {
            node = node->parent;
            return *this;
          }
          else
            node = node->parent;
        }

        node = nullptr;
        return *this;
      }
      IteratorBase operator--(int) { IteratorBase result = *this; --(*this); return result; }

    public:
      NodeType* node;
    };

  public:
    using key_type           = Key;
    using mapped_type        = T;
    using value_type         = Pair<const Key, T>;
    using mutable_value_type = Pair<Key, T>;
    using iterator           = IteratorBase<Node, value_type>;
    using const_iterator     = IteratorBase<const Node, const value_type>;

  public:
    constexpr Map() = default;

  public:
    Map& operator=(Map&& other) = default;
    Map(Map&& other) = default;

  public:
    Map& operator=(const Map& other)
    {
      for(const auto& [key, value] : other)
        insert({key, value});

      return *this;
    }
    Map(const Map& other) { *this = other; }

  public:
    const_iterator begin() const { return m_root ? const_iterator(m_root->leftist()) : const_iterator(nullptr); }
    const_iterator end() const { return const_iterator(nullptr); }
    iterator begin() { return m_root ? iterator(m_root->leftist()) : iterator(nullptr); }
    iterator end() { return iterator(nullptr); }

  public:
    iterator insert(mutable_value_type value)
    {
      if(m_root)
      {
        auto* node = m_root->insert(move(value));
        return iterator(node);
      }
      else
      {
        m_root = makeUnique<Node>(nullptr, nullptr, nullptr, move(value));
        return iterator(m_root.get());
      }
    }

    template<typename K>
    iterator find(const K& key)
    {
      if(m_root)
        return iterator(m_root->find(key));
      else
        return end();
    }

    template<typename K>
    const_iterator find(const K& key) const
    {
      if(m_root)
        return const_iterator(m_root->find(key));
      else
        return end();
    }

    void erase(const_iterator position)
    {
      const_cast<Node*>(position.node)->erase(m_root);
    }

  public:
    bool empty() const { return begin() == end(); }

  private:
    UniquePtr<Node> m_root = nullptr;
  };

  template class Map<int, int>;
}

#pragma once

#include <librt/Assert.hpp>
#include <librt/Iterator.hpp>
#include <librt/Algorithm.hpp>
#include <librt/Utility.hpp>

#include <stddef.h>
#include <type_traits>

namespace rt::containers
{
  template<typename T, size_t N>
  struct StaticVector
  {
  public:
    using value_type      = T;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;
    using iterator        = pointer;
    using const_iterator  = const_pointer;
    using size_type       = size_t;

  public:
    constexpr StaticVector() = default;
    constexpr ~StaticVector() { clear(); }

  public:
    constexpr StaticVector& operator=(const StaticVector& other)
    {
      clear();

      for(const auto& t : other)
        pushBack(t);

      return *this;
    }

    constexpr StaticVector& operator=(StaticVector&& other)
    {
      clear();

      for(auto& t : other)
        pushBack(rt::move(t));

      other.clear();
      return *this;
    }

    constexpr StaticVector(const StaticVector& other) { *this = other; }
    constexpr StaticVector(StaticVector&& other) { *this = rt::move(other); }

  public:
    iterator       begin()        { return reinterpret_cast<iterator>(rt::begin(m_storage)); }
    iterator       end()          { return reinterpret_cast<iterator>(rt::begin(m_storage)+m_size); }
    const_iterator begin()  const { return reinterpret_cast<const_iterator>(rt::begin(m_storage)); }
    const_iterator end()    const { return reinterpret_cast<const_iterator>(rt::begin(m_storage)+m_size); }
    const_iterator cbegin() const { return reinterpret_cast<const_iterator>(rt::begin(m_storage)); }
    const_iterator cend()   const { return reinterpret_cast<const_iterator>(rt::begin(m_storage)+m_size); }

  public:
    void pushBack(T&& t)      { new(end()) T(rt::move(t)); ASSERT(m_size != N); ++m_size; }
    void pushBack(const T& t) { new(end()) T(t);           ASSERT(m_size != N); ++m_size; }
    void popBack() { ASSERT(m_size != 0); --m_size; rt::prev(end())->~T(); }

  public:
    constexpr bool empty() const { return m_size == 0; }
    constexpr size_t size() const { return m_size; }
    constexpr size_t capacity() const { return N; }
    constexpr void resize(size_t size, const T& t = T())
    {
      if(m_size<size)
        while(m_size != size) pushBack(t);
      else if(m_size>size)
        while(m_size != size) popBack();
    }
    constexpr void clear() { resize(0); }

  public:
    reference       operator[](size_type i)       { ASSERT(i<m_size); return reinterpret_cast<reference>(m_storage[i]); }
    const_reference operator[](size_type i) const { ASSERT(i<m_size); return reinterpret_cast<const_reference>(m_storage[i]); }

  public:
    iterator erase(const_iterator it)
    {
      move(next(it), end(), it);
      resize(size()-1);
      return const_cast<iterator>(it);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
      move(const_cast<iterator>(last), end(), const_cast<iterator>(first));
      resize(size()-(last-first));
      return const_cast<iterator>(first);
    }

  private:
    size_t m_size = 0;
    std::aligned_storage_t<sizeof(T), alignof(T)> m_storage[N] = {};
  };
}

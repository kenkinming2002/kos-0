#pragma once

#include <utility>
#include <array>

#include <memory>
#include <algorithm>

namespace utils
{
  template<typename T>
  union Nullable
  {
    constexpr Nullable() : empty() {}

    struct empty_type {} empty;
    T data; // Guranteed to be the largest member
  };

  template<typename T, size_t Capacity>
  class StaticVector
  {
  public:
    using value_type             = T;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using pointer                = value_type*;
    using const_pointer          = const value_type*;
    using iterator               = value_type*;
    using const_iterator         = const value_type*;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  public:
    StaticVector() : m_size(0u) {}

  public:
    constexpr reference operator[](size_type n)             { return m_data[n].data; }
    constexpr const_reference operator[](size_type n) const { return m_data[n].data; }

    constexpr reference front()             { return this->operator[](0); }
    constexpr const_reference front() const { return this->operator[](0); }

    constexpr reference back()             { return this->operator[](m_size-1);}
    constexpr const_reference back() const { return this->operator[](m_size-1);}

    constexpr pointer data()             { return &this->operator[](0); }
    constexpr const_pointer data() const { return &this->operator[](0); }

  public:
    constexpr iterator begin()             { return &this->operator[](0); }
    constexpr const_iterator begin() const { return &this->operator[](0); }
    constexpr iterator end()               { return &this->operator[](m_size);; }
    constexpr const_iterator end() const   { return &this->operator[](m_size);; }

    constexpr const_iterator cbegin() const { return &this->operator[](0); }
    constexpr const_iterator cend() const   { return &this->operator[](m_size);; }

    constexpr reverse_iterator rbegin()             { return &front(); }
    constexpr const_reverse_iterator rbegin() const { return &front(); }
    constexpr reverse_iterator rend()               { return &back(); }
    constexpr const_reverse_iterator rend() const   { return &back(); }

    constexpr const_reverse_iterator crbegin() const { return &front(); }
    constexpr const_reverse_iterator crend() const   { return &back(); }

  public:
    constexpr size_type size() const { return m_size; }
    constexpr size_type capacity() const { return Capacity; }

    constexpr bool empty() const { return size()==0; }

  public:
    void clear() { m_size = 0; }

  public:
    iterator insert(const_iterator pos, const T& value)
    {
      iterator it = &this->operator[](std::distance(cbegin(), pos));

      if(m_size++!=Capacity)
      {
        static_assert(std::is_trivially_destructible_v<T>, "StaticVector only support trivially destructible type");
        std::uninitialized_copy(std::make_reverse_iterator(end()), std::make_reverse_iterator(it),
                                std::make_reverse_iterator(end()+1));
        new (it) T(value);
      }
      return it;
    }

    iterator insert(const_iterator pos, T&& value)
    {
      iterator it = &this->operator[](std::distance(cbegin(), pos));

      if(m_size++!=Capacity)
      {
        static_assert(std::is_trivially_destructible_v<T>, "StaticVector only support trivially destructible type");
        std::uninitialized_copy(std::make_reverse_iterator(end()), std::make_reverse_iterator(it),
                                std::make_reverse_iterator(end()+1));
        new (it) T(std::move(value));
      }
      return it;
    }
    
    template<typename... Args>
    iterator emplace(const_iterator pos, Args&&... args)
    {
      iterator it = &this->operator[](std::distance(cbegin(), pos));

      if(m_size++!=Capacity)
      {
        static_assert(std::is_trivially_destructible_v<T>, "StaticVector only support trivially destructible type");
        std::uninitialized_copy(std::make_reverse_iterator(end()), std::make_reverse_iterator(it),
                                std::make_reverse_iterator(end()+1));
        new (it) T(std::forward<Args>(args)...);
      }
      return it;
    }

    void push_back(const T& value)
    {
      new (end()) T(value);
      ++m_size;
    }

    template<typename... Args>
    void emplace_back(Args&&... args)
    {
      new (end()) T(std::forward<Args>(args)...);
      ++m_size;
    }

    void pop_back() 
    { 
      static_assert(std::is_trivially_destructible_v<T>, "StaticVector only support trivially destructible type");
      --m_size; 
    }

  private:
    Nullable<T> m_data[Capacity];
    size_t m_size;
  };
}

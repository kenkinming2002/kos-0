#pragma once

#include <librt/Utility.hpp>
#include <librt/Algorithm.hpp>
#include <librt/DefaultConstruct.hpp>
#include <librt/NonCopyable.hpp>
#include <librt/Assert.hpp>

#include <cstddef>
#include <type_traits>

namespace rt
{
  template<typename T>
  class UniquePtr : NonCopyable
  {
  public:
    // We do not want to remove extent of bounded array, we need to think about whether direct array support is prefered
    using value_type      = std::conditional_t<std::is_unbounded_array_v<T>, std::remove_extent_t<T>, T>;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = value_type&;
    using const_reference = const value_type&;

  public:
    template<typename>
    friend class UniquePtr;

  public:
    constexpr UniquePtr(pointer ptr = nullptr) { reset(ptr); }
    constexpr ~UniquePtr()                     { reset(); }

  public:
    template<typename U>
    constexpr UniquePtr& operator=(UniquePtr<U>&& other)
    {
      reset(other.release());
      return *this;
    }

    template<typename U>
    constexpr UniquePtr(UniquePtr<U>&& other)
    {
      reset(other.release());
    }

  public:
    constexpr pointer release()
    {
      return exchange(m_ptr, nullptr);
    }

    constexpr void reset(pointer ptr = nullptr) requires (!std::is_unbounded_array_v<T>)
    {
      if(m_ptr)
        delete m_ptr;

      m_ptr = ptr;
    }

    constexpr void reset(pointer ptr = nullptr) requires std::is_unbounded_array_v<T>
    {
      if(m_ptr)
        delete[] m_ptr;

      m_ptr = ptr;
    }

  public:
    constexpr pointer get() { return m_ptr; }
    constexpr const_pointer get() const { return m_ptr; }

  public:
    constexpr pointer operator->()             { ASSERT(*this); return get(); }
    constexpr const_pointer operator->() const { ASSERT(*this); return get(); }

    constexpr reference operator*()             { ASSERT(*this); return *get(); }
    constexpr const_reference operator*() const { ASSERT(*this); return *get(); }

    constexpr operator bool() const { return get(); }

  public:
    constexpr reference operator[](size_t n) requires std::is_unbounded_array_v<T> { return m_ptr[n]; }
    constexpr const_reference operator[](size_t n) const requires std::is_unbounded_array_v<T> { return m_ptr[n]; }

  private:
    pointer m_ptr = nullptr;
  };

  template<typename T>
  static constexpr auto makeUnique(default_construct_t) requires(!std::is_unbounded_array_v<T>)
  {
    return UniquePtr<T>(new typename UniquePtr<T>::value_type);
  }

  template<typename T, typename... Args>
  static constexpr auto makeUnique(Args&&... args) requires(!std::is_unbounded_array_v<T>)
  {
    return UniquePtr<T>(new typename UniquePtr<T>::value_type(forward<Args>(args)...));
  }

  template<typename T>
  static auto makeUnique(size_t size) requires std::is_unbounded_array_v<T>
  {
    return UniquePtr<T>(new typename UniquePtr<T>::value_type[size]);
  }

  template<typename T, typename... Args>
  static auto makeUnique(size_t size, Args&&... args) requires std::is_unbounded_array_v<T>
  {
    auto p =  UniquePtr<T>(new typename UniquePtr<T>::value_type[size]);
    if(p)
      fill(&p[0], &p[size], typename UniquePtr<T>::value_type(rt::forward<Args>(args)...));

    return move(p);
  }
}

#pragma once

#include <type_traits>

#define FORCE_INLINE [[gnu::always_inline]]

namespace utils
{
  template<typename T>
  FORCE_INLINE constexpr typename std::remove_reference<T>::type&& move(T&& t ) noexcept
  {
    return static_cast<typename std::remove_reference<T>::type&&>(t);
  }

  template <class T>
  FORCE_INLINE constexpr T&& forward(typename std::remove_reference<T>::type& t) noexcept
  {
      return static_cast<T&&>(t);
  }

  template <class T>
  FORCE_INLINE constexpr T&& forward(typename std::remove_reference<T>::type&& t) noexcept
  {
      return static_cast<T&&>(t);
  }

  template<typename T, typename pointer>
  FORCE_INLINE constexpr auto& deref_cast(pointer ptr) noexcept
  {
    return *reinterpret_cast<T*>(ptr);
  }

  template<typename It>
  FORCE_INLINE constexpr auto iterator_cast(It it) noexcept
  {
    return &(*it);
  }

  template< class T, std::size_t N >
  FORCE_INLINE constexpr T* begin( T (&array)[N] ) noexcept
  {
    return &array[0];
  }

  template< class T, std::size_t N >
  FORCE_INLINE constexpr T* end( T (&array)[N] ) noexcept
  {
    return &array[N];
  }

  template<typename RandomAccessIterator>
  FORCE_INLINE constexpr auto distance(RandomAccessIterator first, RandomAccessIterator last)
  {
    return last - first;
  }
}

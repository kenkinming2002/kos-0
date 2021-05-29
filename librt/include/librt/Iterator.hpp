#pragma once

#include <stddef.h>
#include <utility>

namespace rt
{
  template<typename BidirectionalIterator>
  BidirectionalIterator prev(BidirectionalIterator it)
  {
    --it;
    return it;
  }

  template<typename InputIterator>
  InputIterator next(InputIterator it)
  {
    ++it;
    return it;
  }

  template<typename T, size_t N>
  constexpr T* begin( T(&array)[N] ) { return array; }

  template<typename T, size_t N>
  constexpr T* end( T(&array)[N] ) { return array+N; }

  template<typename T, size_t N>
  constexpr size_t size( T(&array)[N] ) { return N; }

  template<typename Iterator>
  class ReverseIterator
  {
  public:
    constexpr auto& operator*()             { auto tmp = m_it; return *(--tmp); }
    constexpr const auto& operator*() const { auto tmp = m_it; return *(--tmp); }

    constexpr auto* operator->()             { return &(this->operator*()); }
    constexpr const auto* operator->() const { return &(this->operator*()); }

    constexpr ReverseIterator(Iterator it) : m_it(it) {}

    constexpr ReverseIterator& operator--()    { m_it++; return *this; }
    constexpr ReverseIterator& operator++()    { m_it--; return *this; }

    constexpr ReverseIterator  operator--(int) { return ReverseIterator(++m_it); }
    constexpr ReverseIterator  operator++(int) { return ReverseIterator(--m_it); }

    template<typename DifferenceType> constexpr ReverseIterator operator-=(DifferenceType n) { m_it+=n; return *this; }
    template<typename DifferenceType> constexpr ReverseIterator operator+=(DifferenceType n) { m_it-=n; return *this; }

    template<typename DifferenceType> friend constexpr ReverseIterator operator-(ReverseIterator it, DifferenceType n) { return ReverseIterator(it.m_it+n); }
    template<typename DifferenceType> friend constexpr ReverseIterator operator+(ReverseIterator it, DifferenceType n) { return ReverseIterator(it.m_it-n); }
    friend constexpr auto operator-(ReverseIterator it1, ReverseIterator it2) { return it2.m_it-it1.m_it; }

  public:
    friend auto operator<=>(const ReverseIterator& lhs, const ReverseIterator& rhs) = default;

  private:
    Iterator m_it;
  };

  template class ReverseIterator<int*>;
}

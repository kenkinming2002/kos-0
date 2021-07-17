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
    constexpr ReverseIterator() = default;
    constexpr ReverseIterator(Iterator it) : m_it(it) {}

  public:
    constexpr auto& operator*()             { auto tmp = m_it; return *(--tmp); }
    constexpr const auto& operator*() const { auto tmp = m_it; return *(--tmp); }

    constexpr auto* operator->()             { return &(this->operator*()); }
    constexpr const auto* operator->() const { return &(this->operator*()); }

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

  template<typename Iterator>
  class MutableIterator
  {
  public:
    constexpr MutableIterator(Iterator it) : m_it(it) {}

  public:
    Iterator base() const { return m_it; }
    operator Iterator() const { return base(); }

  public:
    template<typename T>
    operator MutableIterator<T>() const { return MutableIterator<T>(T(m_it)); }

  private:
    using value_type      = std::remove_const_t<std::remove_reference_t<decltype(*std::declval<Iterator>())>>;
    using reference       = value_type&;
    using pointer         = value_type*;

  public:
    constexpr reference operator*()  { return const_cast<reference>(*m_it); }
    constexpr pointer   operator->() { return const_cast<pointer>(&(*m_it)); }

  public:
    friend auto operator<=>(const MutableIterator& lhs, const MutableIterator& rhs) = default;

  // Bidirectional iterator
  public:
    constexpr MutableIterator& operator++() { m_it++; return *this; }
    constexpr MutableIterator& operator--() { m_it--; return *this; }

    constexpr MutableIterator operator++(int) { return MutableIterator(++m_it); }
    constexpr MutableIterator operator--(int) { return MutableIterator(--m_it); }

  // Random access iterator
  //private:
  //  using difference_type = decltype(std::declval<Iterator>()-std::declval<Iterator>());

  //public:
  //  constexpr MutableIterator& operator+=(difference_type n) { m_it+=n; return *this; }
  //  constexpr MutableIterator& operator-=(difference_type n) { m_it-=n; return *this; }

  //  constexpr MutableIterator operator+(difference_type n) const { return MutableIterator(static_cast<Iterator>(*this)+n); }
  //  constexpr MutableIterator operator-(difference_type n) const { return MutableIterator(static_cast<Iterator>(*this)-n); }

  //  friend constexpr auto operator-(const MutableIterator& lhs, const MutableIterator& rhs) { return MutableIterator(static_cast<Iterator>(lhs)-static_cast<Iterator>(rhs)); }

  private:
    Iterator m_it;
  };

  template class MutableIterator<int*>;
  template class ReverseIterator<int*>;

  template<typename Iterator,typename DifferenceType>
  Iterator next(Iterator it, DifferenceType difference)
  {
    for(DifferenceType i = 0; i != difference; ++i)
      ++it;

    return it;
  }
}

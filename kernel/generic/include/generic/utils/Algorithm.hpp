#pragma once

#include <stddef.h>

namespace utils
{
  struct Sentinel {};

  template<typename It>
  struct take_iterator
  {
  private:
    It        m_begin, m_end;
    ptrdiff_t m_count;

  public:
    take_iterator(It begin, It end, ptrdiff_t count) : m_begin(begin), m_end(end), m_count(count) {}

  public:
    explicit operator It() { return m_begin; }

  public:
    auto& operator++() { --m_count; ++m_begin; return *this; }
    auto& operator--() { ++m_count; --m_begin; return *this; }

    auto& operator++(int) { auto copy = *this; ++*this; return copy; }
    auto& operator--(int) { auto copy = *this; --*this; return copy; }

    auto& operator*() const { return *m_begin; }
    auto operator->() const { return m_begin; }

  public:
    bool operator==(Sentinel) const { return m_count<=0 || m_begin==m_end; }
    bool operator!=(Sentinel) const { return m_count>0  && m_begin!=m_end; }
  };

  template<typename It>
  struct take_range
  {
  public:
    take_range(It begin, It end, ptrdiff_t count) : m_it(begin, end, count) {}

  public:
    auto begin() const { return m_it; }
    auto end()   const { return Sentinel{}; }

  private:
    take_iterator<It> m_it;
  };

  template<typename InputIt, typename Sentinel, typename Func>
  constexpr void for_each(InputIt first, Sentinel last, Func func)
  {
    for(; first!=last; ++first)
      func(*first);
  }

  template<typename InputIt, typename Sentinel, typename UnaryPredicate>
  constexpr InputIt find(InputIt first, Sentinel last, UnaryPredicate p)
  {
    for(; first!=last; ++first)
      if(p(*first))
        break;
    return first;
  }

  template<typename InputIt, typename Sentinel, typename UnaryPredicate>
  constexpr InputIt find(InputIt first, Sentinel last, UnaryPredicate p, size_t count)
  {
    // 1: Find Element Satisfying Predicate
    while(first!=last)
    {
      first = find(first, last, p);
      // 2: Check if the next (count-1) Elements Satisfy the Predicate
      auto ret = first;
      for(size_t i=1;; ++i, ++first)
      {
        if(i>=count)
          return ret;

        if(!p(*first))
          break;
      }
    }
    return first;
  }


  template<typename InputIt, typename Sentinel, typename UnaryPredicate>
  constexpr bool all_of(InputIt first, Sentinel last, UnaryPredicate p)
  {
    for(; first!=last; ++first)
      if(!p(*first))
        return false;

    return true;
  }
}

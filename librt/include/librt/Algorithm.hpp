#pragma once

#include <librt/Iterator.hpp>
#include <librt/Utility.hpp>

namespace rt
{
  template<typename InputIterator, typename OutputIterator>
  constexpr OutputIterator copy(InputIterator first, InputIterator last, OutputIterator dest)
  {
    while(first != last)
      *dest++ = *first++;

    return dest;
  }

  template<typename InputIterator, typename OutputIterator>
  constexpr OutputIterator copy_n(InputIterator first, OutputIterator dest, size_t n)
  {
    for(size_t i=0; i<n; ++i)
      *dest++ = *first++;

    return dest;
  }

  template<typename InputIterator, typename OutputIterator>
  constexpr OutputIterator move(InputIterator first, InputIterator last, OutputIterator dest)
  {
    while(first != last)
      *dest++ = move(*first++);

    return dest;
  }

  template<typename ForwardIterator, typename T>
  constexpr void fill(ForwardIterator first, ForwardIterator last, const T& value)
  {
    while(first != last)
      *first++ = value;
  }

  template<typename InputIterator, typename T >
  constexpr InputIterator find(InputIterator first, InputIterator last, const T& value)
  {
    for(; first != last; ++first)
      if(*first == value)
        return first;

    return last;
  }

  template<typename InputIterator, typename UnaryPredicate >
  constexpr InputIterator find_if(InputIterator first, InputIterator last, UnaryPredicate p)
  {
    for(; first != last; ++first)
      if(p(*first))
        return first;

    return last;
  }

  template<typename RandomAccessIterator, typename Compare>
  constexpr void sort(RandomAccessIterator first, RandomAccessIterator last, Compare compare)
  {
    if(first == last)
      return;

    // Bubble Sort
    for(auto it1 = first; it1 != prev(last); ++it1)
      for(auto it2 = it1; it2 != prev(last); ++it2)
      {
        if(compare(*next(it2), *it2))
          swap(*next(it2), *it2);
      }
  }

  template<typename ForwardIterator, typename Compare, typename Merge>
  constexpr ForwardIterator adjacentMerge(ForwardIterator first, ForwardIterator last, Compare compare, Merge merge)
  {
    if(first == last)
      return last;

    ForwardIterator output = first;
    for(auto it = next(first); it != last; ++it)
      if(compare(*output, *it))
      {
        *output = merge(*output, *it);
      }
      else
      {
        ++output;
        *output = *it;
      }

    ++output;
    return output;
  }

  template<typename InputIterator1, typename InputIterator2>
  constexpr int lexicographicalCompare(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2)
  {
    for(;first1 != last1 && first2 != last2; ++first1, ++first2)
    {
      if(*first1 != *first2)
        return *first1 < *first2 ? -1 : 1;
    }
    if(first1 != last1)
      return 1;

    if(first2 != last2)
      return -1;

    return 0;
  }

  template<typename inputiterator, typename unarypredicate>
  constexpr bool all(inputiterator first, inputiterator last, unarypredicate p)
  {
    for(auto it = first; it!=last; ++it)
      if(!p(*it))
        return false;

    return true;
  }

  template<typename inputiterator, typename unarypredicate>
  constexpr bool any(inputiterator first, inputiterator last, unarypredicate p)
  {
    for(auto it = first; it!=last; ++it)
      if(p(*it))
        return true;

    return false;
  }

  template<typename inputiterator, typename unarypredicate>
  constexpr bool none(inputiterator first, inputiterator last, unarypredicate p)
  {
    for(auto it = first; it!=last; ++it)
      if(p(*it))
        return false;

    return true;
  }
}

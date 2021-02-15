#pragma once

#include <librt/Iterator.hpp>
#include <librt/Utility.hpp>

namespace rt
{
  template<typename InputIterator, typename OutputIterator>
  OutputIterator copy(InputIterator first, InputIterator last, OutputIterator dest)
  {
    while(first != last)
      *dest++ = *first++;

    return dest;
  }

  template<typename ForwardIterator, typename T>
  void fill(ForwardIterator first, ForwardIterator last, const T& value)
  {
    while(first != last)
      *first++ = value;
  }

  template<typename InputIterator, typename UnaryPredicate >
  InputIterator find_if(InputIterator first, InputIterator last, UnaryPredicate p)
  {
    for(; first != last; ++first)
      if(p(*first))
        return first;

    return last;
  }

  template<typename RandomAccessIterator, typename Compare>
  void sort(RandomAccessIterator first, RandomAccessIterator last, Compare compare)
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
  ForwardIterator adjacentMerge(ForwardIterator first, ForwardIterator last, Compare compare, Merge merge)
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
}

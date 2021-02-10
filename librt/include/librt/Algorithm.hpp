#pragma once

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
}

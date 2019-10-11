#pragma once

#include <new>
#include <type_traits>
#include <functional>

template<typename T>
struct defer
{
public:
  template<typename... Args>
  void construct(Args&&... args)
  {
    new (&m_storage) T(std::forward<Args&&>(args)...);
  }

public:
  operator T&() { return *std::launder(&value); }
  operator const T&() const{ return *std::launder(&value); }

private:
  std::aligned_storage_t<T, std::alignment_of<T>> m_storage;
};

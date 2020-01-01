#pragma once

#include <utils/Utilities.hpp>

namespace utils
{
  template<typename T>
  constexpr auto NullDeleter = [](T*) {};

  template<typename T, typename Deleter = decltype(NullDeleter<T>)>
  class UniquePointer
  {
  public:
    UniquePointer(Deleter deleter = Deleter()) : m_ptr(nullptr), m_deleter(deleter) {}
    UniquePointer(std::nullptr_t, Deleter deleter = Deleter()) : m_ptr(nullptr), m_deleter(deleter) {}
    UniquePointer& operator=(std::nullptr_t) { this->reset(nullptr); }

  public:
    UniquePointer(T* ptr, Deleter deleter = Deleter()) : m_ptr(ptr), m_deleter(deleter) {}

  public:
    UniquePointer(UniquePointer&& other) : m_ptr(other.release()), m_deleter(utils::move(other.m_deleter)) { other.m_ptr = nullptr; }
    UniquePointer& operator=(UniquePointer&& other)
    {
      this->reset(other.release());
      return *this;
    }

  public:
    T& operator*() const { return *m_ptr; }
    T* operator->() const { return m_ptr; }

  public:
    T* get() const noexcept { return m_ptr; }

    void reset(T* ptr)
    {
      auto old_ptr = m_ptr;
      m_ptr = ptr;
      if(old_ptr)
        m_deleter(old_ptr);
    }

    T* release()
    {
      auto ptr = m_ptr;
      m_ptr = nullptr;
      return ptr;
    }

  private:
    T* m_ptr;
    [[no_unique_address]]Deleter m_deleter;
  };
}

#pragma once

#include <librt/Utility.hpp>

#include <cstddef>

namespace rt
{
  template<typename T>
  class UniquePtr
  {
  public:
    using pointer = T*;
    using element_type = T;

  public:
    constexpr UniquePtr() : m_ptr(nullptr) {}
    constexpr UniquePtr(std::nullptr_t) : m_ptr(nullptr) {}
    constexpr UniquePtr(T* ptr) : m_ptr(ptr) {}

  public:
    UniquePtr& operator=(UniquePtr&& other) { m_ptr = rt::exchange(other.m_ptr, nullptr); return *this; }
    UniquePtr(UniquePtr&& other) : UniquePtr() { *this = rt::move(other); }

    UniquePtr& operator=(const UniquePtr& other) = delete;
    UniquePtr(const UniquePtr& other) = delete;

  public:
    ~UniquePtr() { delete m_ptr; }

  public:
    T* get() { return m_ptr; }
    const T* get() const { return m_ptr; }

  public:
    T* operator->() { return m_ptr; }
    const T* operator->() const { return m_ptr; }

    T& operator*() { return *m_ptr; }
    const T& operator*() const { return *m_ptr; }

    operator bool() const { return m_ptr; }

  private:
    T* m_ptr = nullptr;
  };

  template<typename T, typename... Args>
  UniquePtr<T> makeUnique(Args&&... args)
  {
    return UniquePtr<T>(new T(rt::forward<Args>(args)...));
  }
}

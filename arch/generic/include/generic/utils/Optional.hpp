#pragma once

#include <type_traits>

namespace utils
{
  struct NullOptional
  {
    explicit constexpr NullOptional(int) {}
  };
  constexpr auto nullopt = NullOptional(0);

  template<typename T>
  struct Optional
  {
  public:
    Optional() : m_initialized(false) {}
    Optional(NullOptional) : m_initialized(false) {}

    Optional(const T& value) : m_initialized(true)
    {
      this->value() = value;
    }
    Optional(T&& value) : m_initialized(true)
    {
      this->value() = value;
    }

    Optional(const Optional& other) : m_initialized(true)
    {
      this->value() = other.value();
    }

    Optional(Optional&& other) : m_initialized(true)
    {
      this->value() = other.value();
    }

  public:
    const T* operator->() const { return reinterpret_cast<const T*>(&m_storage); }
    T* operator->() { return reinterpret_cast<T*>(&m_storage); }

    const T& operator*() const { return *reinterpret_cast<const T*>(&m_storage); }
    T& operator*() { return *reinterpret_cast<T*>(&m_storage); }

  public:
    const T& value() const { return *reinterpret_cast<const T*>(&m_storage); }
    T& value() { return *reinterpret_cast<T*>(&m_storage); }

  public:
    bool has_value() const { return m_initialized; }
    operator bool() const { return has_value(); }

  public:
    ~Optional() { value().~T(); }

  private:
    bool m_initialized;
    std::aligned_storage_t<sizeof(T), alignof(T)> m_storage;
  };
}

#pragma once

#include <librt/Utility.hpp>
#include <librt/DefaultConstruct.hpp>
#include <librt/NonCopyable.hpp>
#include <librt/Assert.hpp>

#include <atomic>
#include <cstddef>
#include <type_traits>

namespace rt
{
  class SharedPtrHook : public rt::NonCopyable
  {
    template<typename> friend class SharedPtr;

    /* We use size_t to prepare for the worse case.  Since each objects
     * managed by SharedPtr is at least one bytes, there is no way for count
     * to exceed the maximum value representable by size_t before exhausting
     * all the memory.  */
    std::atomic<size_t> count = 1;
  };

  template<typename T>
  class SharedPtr : NonCopyable
  {
  public:
    static_assert(std::is_base_of_v<SharedPtrHook, T>);

  public:
    using value_type      = T;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = value_type&;
    using const_reference = const value_type&;

  public:
    template<typename>
    friend class SharedPtr;

  public:
    explicit constexpr SharedPtr(T* ptr) { reset(ptr); }

    constexpr SharedPtr()               : SharedPtr(nullptr) {}
    constexpr SharedPtr(std::nullptr_t) : SharedPtr(static_cast<T*>(nullptr)) {}
    constexpr ~SharedPtr()                { reset(); }

  public:
    constexpr SharedPtr& operator=(SharedPtr&& other) { reset(other.release()); return *this; }
    constexpr SharedPtr(SharedPtr&& other)            { reset(other.release()); }

    template<typename U> constexpr SharedPtr& operator=(SharedPtr<U>&& other) { reset(other.release()); return *this; }
    template<typename U> constexpr SharedPtr(SharedPtr<U>&& other)            { reset(other.release()); }

    constexpr SharedPtr& operator=(const SharedPtr& other) { reset(other.acquire()); return *this; }
    constexpr SharedPtr(const SharedPtr& other)            { reset(other.acquire()); }

    template<typename U> constexpr SharedPtr& operator=(const SharedPtr<U>& other) { reset(other.acquire()); return *this; }
    template<typename U> constexpr SharedPtr(const SharedPtr<U>& other)            { reset(other.acquire()); }

  public:
    constexpr T* acquire() const
    {
      if(m_ptr)
        ++static_cast<SharedPtrHook*>(m_ptr)->count;

      return m_ptr;
    }

    constexpr T* release()
    {
      return exchange(m_ptr, nullptr);
    }

    constexpr void reset(T* ptr = nullptr)
    {
      /* There is no overflow here, because we are actually holding a reference
       * count */
      if(m_ptr && --static_cast<SharedPtrHook*>(m_ptr)->count == 0)
        delete m_ptr;

      m_ptr = ptr;
      ASSERT(!m_ptr || static_cast<SharedPtrHook*>(m_ptr)->count != 0);
    }

  public:
    constexpr pointer get()             { return m_ptr; }
    constexpr const_pointer get() const { return m_ptr; }

  public:
    constexpr pointer operator->()             { ASSERT(*this); return get(); }
    constexpr const_pointer operator->() const { ASSERT(*this); return get(); }

    constexpr reference operator*()             { ASSERT(*this); return *get(); }
    constexpr const_reference operator*() const { ASSERT(*this); return *get(); }

    constexpr operator bool() const { return get(); }

  public:
    unsigned count() const { return static_cast<SharedPtrHook*>(m_ptr)->count; }

  private:
    T* m_ptr = nullptr;
  };


  template<typename T>
  static constexpr auto makeShared(default_construct_t) requires(!std::is_unbounded_array_v<T>)
  {
    return SharedPtr<T>(new typename SharedPtr<T>::value_type);
  }

  template<typename T, typename... Args>
  static constexpr auto makeShared(Args&&... args) requires(!std::is_unbounded_array_v<T>)
  {
    return SharedPtr<T>(new typename SharedPtr<T>::value_type(forward<Args>(args)...));
  }

  template<typename T>
  static auto makeShared(size_t size) requires std::is_unbounded_array_v<T>
  {
    return SharedPtr<T>(new typename SharedPtr<T>::value_type[size]);
  }
}

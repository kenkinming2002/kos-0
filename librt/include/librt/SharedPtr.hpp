#pragma once

#include <librt/Utility.hpp>
#include <librt/DefaultConstruct.hpp>
#include <librt/NonCopyable.hpp>
#include <librt/Assert.hpp>

#include <atomic>
#include <type_traits>

#include <stddef.h>

namespace rt
{
  struct SharedPtrHook : public rt::NonCopyable
  {
    template<typename> friend class SharedPtr;

    /* We use size_t to prepare for the worse case.  Since each objects
     * managed by SharedPtr is at least one bytes, there is no way for count
     * to exceed the maximum value representable by size_t before exhausting
     * all the memory.  */
    std::atomic<size_t> count = 0;
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
    constexpr void adopt(pointer ptr) { ASSERT(m_ptr == nullptr); m_ptr = ptr; }
    constexpr reference release()     { ASSERT(m_ptr != nullptr); return *exchange(m_ptr, nullptr); }

  public:
    constexpr void addRef()
    {
      if(m_ptr)
        ++static_cast<SharedPtrHook*>(m_ptr)->count;
    }

    constexpr void dropRef()
    {
      if(m_ptr)
        if(--static_cast<SharedPtrHook*>(m_ptr)->count == 0)
          delete m_ptr;
    }

  public:
    explicit constexpr SharedPtr(pointer ptr) { m_ptr = ptr; addRef(); }
    constexpr ~SharedPtr()                    { dropRef(); m_ptr = nullptr;}

  public:
    constexpr SharedPtr()               : SharedPtr(nullptr) {}
    constexpr SharedPtr(std::nullptr_t) : SharedPtr(static_cast<T*>(nullptr)) {}

  public:
    constexpr SharedPtr& operator=(SharedPtr&& other)      { dropRef(); m_ptr = exchange(other.m_ptr, nullptr); return *this; }
    constexpr SharedPtr& operator=(const SharedPtr& other) { dropRef(); m_ptr = other.m_ptr; this->addRef();    return *this; }

    template<typename U> constexpr SharedPtr& operator=(SharedPtr<U>&& other)      { dropRef(); m_ptr = static_cast<pointer>(exchange(other.m_ptr, nullptr)); return *this; }
    template<typename U> constexpr SharedPtr& operator=(const SharedPtr<U>& other) { dropRef(); m_ptr = static_cast<pointer>(other.m_ptr); this->addRef();    return *this; }

  public:
    constexpr SharedPtr(SharedPtr&& other)      : SharedPtr() { *this = move(other); }
    constexpr SharedPtr(const SharedPtr& other) : SharedPtr() { *this = other; }

    template<typename U> constexpr SharedPtr(SharedPtr<U>&& other)      : SharedPtr() { *this = move(other); }
    template<typename U> constexpr SharedPtr(const SharedPtr<U>& other) : SharedPtr() { *this = other; }

  public:
    constexpr void reset() { dropRef(); m_ptr = nullptr; } // Synonym of dropRef

  public:
    constexpr pointer get()             { return m_ptr; }
    constexpr const_pointer get() const { return m_ptr; }

  public:
    constexpr pointer operator->()             { ASSERT(*this); ASSERT(count() != 0); return get(); }
    constexpr const_pointer operator->() const { ASSERT(*this); ASSERT(count() != 0); return get(); }

    constexpr reference operator*()             { ASSERT(*this); ASSERT(count() != 0); return *get(); }
    constexpr const_reference operator*() const { ASSERT(*this); ASSERT(count() != 0); return *get(); }

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

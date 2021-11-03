#pragma once

#include <librt/Utility.hpp>

namespace rt
{
  template<typename>
  class FunctionRef;

  template<typename Ret, typename... T>
  class FunctionRef<Ret(T...)>
  {
  public:
    using func_ptr_t         = Ret(*)(T...);
    using func_capture_ptr_t = Ret(*)(T..., void*);

  private:
    template<typename U>
    static func_capture_ptr_t makeFuncCapture()
    {
      return [](T... args, void* capture){
        U& u = *static_cast<U*>(capture);
        return u(rt::move(args)...);
      };
    }

  private:
    union
    {
      func_ptr_t         m_func = nullptr; // If !m_capture
      func_capture_ptr_t m_func_capture;   // If m_capture
    };
    void* m_capture = nullptr;

  public:
    constexpr FunctionRef() = default;
    /* Free Function */
    template<typename U>
    constexpr FunctionRef(U u) requires(std::is_convertible_v<U, func_ptr_t>)
      : m_func(u), m_capture(nullptr) {}

    template<typename U>
    constexpr FunctionRef(U& u) requires(!std::is_convertible_v<U, func_ptr_t>)
      : m_func_capture(makeFuncCapture<U>()), m_capture(&u) {}

  public:
    Ret operator()(T&&... args) const
    {
      if(m_capture)
        return m_func_capture(m_capture, forward<T>(args)...);
      else
        return m_func(forward<T>(args)...);
    }
  };
}

#pragma once

#include <type_traits>

namespace utils
{
  class Callback
  {
  public:
    using ptr_t = void(*)();
    using ptr_data_t = void(*)(void*);

  public:
    Callback() : m_ptr(nullptr), m_data(nullptr) {}

    template<typename Callable, typename = std::enable_if_t<std::is_function_v<Callable>>>
    Callback(Callable& callable) :
      m_ptr_data([](void* callable){ static_cast<Callable&>(*callable)(); }),
      m_data(&callable) {}

    Callback(void(*ptr)(void)) :
      m_ptr(ptr),
      m_data(nullptr) {}

  public:
    void operator()() const 
    { 
      if(m_data)
        m_ptr_data(m_data); 
      else
        m_ptr();
    }

  public:
    operator bool() const { return m_ptr; }

  private:
    union
    {
      ptr_data_t m_ptr_data;
      ptr_t      m_ptr;
    };
    void* m_data;
  };
}

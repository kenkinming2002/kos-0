#pragma once

#include <librt/Utility.hpp>
#include <librt/Assert.hpp>

#include <type_traits>
#include <new>

#include <stddef.h>
#include <utility>

namespace rt
{
  template<typename T> struct type_constant_t{};
  template<typename T> constexpr auto type_constant = type_constant_t<T>{};

  template<typename T, T t> struct integral_constant_t{};
  template<typename T, T t> constexpr auto integral_constant = integral_constant_t<T, t>{};

  template<typename U, typename... Ts> struct index_helper;
  template<typename U, typename... Ts> constexpr static size_t index_helper_v = index_helper<U, Ts...>::value;

  template<typename U, typename... Ts>
  struct index_helper<U, U, Ts...> { constexpr static size_t value = 0; };
  template<typename U, typename T, typename... Ts>
  struct index_helper<U, T, Ts...> { constexpr static size_t value = index_helper<U, Ts...>::value+1; };

  template<size_t I, typename... Ts> struct type_helper;
  template<size_t I, typename... Ts> using type_helper_t = typename type_helper<I, Ts...>::type;

  template<typename T, typename... Ts>
  struct type_helper<0, T, Ts...> { using type = T; };
  template<size_t I, typename T, typename... Ts>
  struct type_helper<I, T, Ts...> { using type = type_helper_t<I-1, Ts...>; };

  static_assert(index_helper<const char*, const char*, int, char*>::value == 0);
  static_assert(index_helper<int        , const char*, int, char*>::value == 1);
  static_assert(index_helper<char*      , const char*, int, char*>::value == 2);

  static_assert(std::is_same_v<type_helper<0, const char*, int, char*>::type, const char*>);
  static_assert(std::is_same_v<type_helper<1, const char*, int, char*>::type, int        >);
  static_assert(std::is_same_v<type_helper<2, const char*, int, char*>::type, char*      >);

  template<typename... Ts>
  struct Variant
  {
  private:
    static constexpr size_t nullIndex = sizeof...(Ts);

  public:
    bool empty() const { return m_index == nullIndex; }
    template<typename T> bool holds() const { return m_index == index_helper_v<T, Ts...>; }
    template<typename T> const T& get() const
    {
      ASSERT(holds<T>());
      return reinterpret_cast<const T&>(m_storage);
    }
    template<typename T> T& get() { return const_cast<T&>(asConst(*this).template get<T>()); }

  public:
    template<typename T, typename Visitor>
    void visitHelper(Visitor visitor) { if(holds<T>()) visitor(get<T>()); };
    template<typename Visitor>
    void visit(Visitor visitor) { (visitHelper<Ts>(visitor), ...); }

    template<typename T, typename Visitor>
    void visitHelper(Visitor visitor) const { if(holds<T>()) visitor(get<T>()); };
    template<typename Visitor>
    void visit(Visitor visitor) const { (visitHelper<Ts>(visitor), ...); }

  public:
    void reset()
    {
      visit([](auto& t){
        using T = std::remove_cvref_t<decltype(t)>;
        t.~T();
      });
      m_index = nullIndex;
    }

    template<typename T, typename... Args>
    void emplace(type_constant_t<T>, Args&&... args)
    {
      reset();
      new(&m_storage) T(forward<Args>(args)...);
      m_index = index_helper_v<T, Ts...>;
    }

  public:
    constexpr Variant() : m_index(nullIndex), m_storage{} {}
    template<typename T, typename... Args>
    Variant(type_constant_t<T> type, Args&&... args) : Variant() { emplace(type, std::forward<Args>(args)...); }

    Variant& operator=(Variant&& other)
    {
      reset();
      other.visit([this](auto& t){
        using T = std::remove_cvref_t<decltype(t)>;
        new(&m_storage) T(move(t));
      });
      other.reset();
      return *this;
    }

    Variant& operator=(const Variant& other)
    {
      reset();
      other.visit([this](const auto& t){
        using T = std::remove_cvref_t<decltype(t)>;
        new(&m_storage) T(t);
      });
      return *this;
    }

    Variant(Variant&& other) : Variant() { *this = move(other); }
    Variant(const Variant& other) : Variant() { *this = other; }

    ~Variant() { reset(); }

  private:
    size_t m_index;
    std::aligned_storage_t<max(sizeof(Ts)...), max(alignof(Ts)...)> m_storage;
  };

  template struct Variant<int, size_t>;
}

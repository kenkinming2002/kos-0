#pragma once

#include "librt/NonCopyable.hpp"
#include <librt/StringRef.hpp>

#include <librt/UniquePtr.hpp>
#include <librt/Algorithm.hpp>

#include <type_traits>

namespace rt
{
  template<typename Char>
  class StringBase
  {
  public:
    explicit StringBase(StringRefBase<Char> stringRef) : StringBase(stringRef.data(), stringRef.length()) {}
    operator StringRefBase<Char>() const & { return StringRefBase<Char>(data(), length()); }
    StringRefBase<Char> ref() const & { return StringRefBase<Char>(data(), length()); }

  public:
    StringBase(size_t length, char c)          : m_buf(makeUnique<Char[]>(length)), m_length(length), m_capacity(length) { fill(begin(), end(), c); }
    StringBase(const Char* str, size_t length) : m_buf(makeUnique<Char[]>(length)), m_length(length), m_capacity(length) { copy(str, str+m_length, begin()); }
    StringBase(const Char* str) : StringBase(str, strlen(str)) { }
    StringBase(const Char* begin, const Char* end) : StringBase(begin, end-begin) {}

  public:
    constexpr StringBase() = default;
    constexpr ~StringBase() = default;
    StringBase(StringBase&& other) = default;
    StringBase& operator=(StringBase&& other) = default;

  public:
    Char& operator[](size_t i) { return data()[i]; }
    const Char& operator[](size_t i) const { return data()[i]; }

  public:
    constexpr const Char* data() const { return m_buf.get(); }
    constexpr Char* data() { return m_buf.get(); }

    constexpr size_t length()    const { return m_length; }

  public:
    constexpr auto begin() { return data(); }
    constexpr auto end()   { return data()+length(); }

    constexpr auto begin() const { return data(); }
    constexpr auto end()   const { return data()+length(); }

    constexpr auto cbegin() const { return data(); }
    constexpr auto cend()   const { return data()+length(); }

    constexpr auto rbegin() { return ReverseIterator(end()); }
    constexpr auto rend()   { return ReverseIterator(begin()); }

    constexpr auto rbegin() const { return ReverseIterator(end()); }
    constexpr auto rend()   const { return ReverseIterator(begin()); }

    constexpr auto crbegin() const { return ReverseIterator(cend()); }
    constexpr auto crend()   const { return ReverseIterator(cbegin()); }

  public:
    constexpr bool empty() const { return length() == 0; }

  private:
    UniquePtr<Char[]> m_buf = nullptr;
    size_t m_length = 0, m_capacity = 0; // Length and capacity of the string excluding the null terminator
  };

  // We don't want implicit conversion
  template<typename Char> auto operator<=>(const StringBase<Char>& lhs, const StringBase<Char>& rhs) { return lhs.ref() <=> rhs.ref(); }
  template<typename Char> bool operator!= (const StringBase<Char>& lhs, const StringBase<Char>& rhs) { return lhs.ref() !=  rhs.ref(); }
  template<typename Char> bool operator== (const StringBase<Char>& lhs, const StringBase<Char>& rhs) { return lhs.ref() ==  rhs.ref(); }

  using String = StringBase<char>;
}

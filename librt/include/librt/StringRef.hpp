#pragma once

#include "librt/Algorithm.hpp"
#include <librt/Utility.hpp>
#include <librt/Iterator.hpp>

#include <librt/Strings.hpp>

#include <stddef.h>

namespace rt
{
  template<typename Char>
  class StringRefBase
  {
  public:
    constexpr StringRefBase() = default;
    constexpr StringRefBase(const Char* str, size_t length) : m_str(str), m_length(length) {}
    constexpr StringRefBase(const Char* str) : StringRefBase(str, strlen(str)) {}
    constexpr StringRefBase(const Char* begin, const Char* end) : StringRefBase(begin, end-begin) {}

  public:
    constexpr const Char& operator[](size_t i) const { return data()[i]; }

  public:
    constexpr const Char* data()   const { return m_str; }
    constexpr size_t      length() const { return m_length; }

  public:
    constexpr auto begin() const { return data(); }
    constexpr auto end()   const { return data()+length(); }

    constexpr auto rbegin() const { return ReverseIterator(end()); }
    constexpr auto rend()   const { return ReverseIterator(begin()); }

  public:
    constexpr bool empty() const { return length() == 0; }

  public:
    [[nodiscard]] constexpr auto substr(size_t pos, size_t count) const { return StringRefBase(m_str+pos, min(count, m_length-pos)); }
    [[nodiscard]] constexpr auto removePrefix(size_t count) const { return substr(count, m_length-count); }
    [[nodiscard]] constexpr auto removeSuffix(size_t count) const { return substr(0    , m_length-count); }

  public:
    static int compare(const StringRefBase& lhs, const StringRefBase& rhs)
    {
      return rt::lexicographicalCompare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

  public:
    friend auto operator<=>(const StringRefBase& lhs, const StringRefBase& rhs) { return compare(lhs, rhs) <=> 0; }
    friend bool operator!= (const StringRefBase& lhs, const StringRefBase& rhs) { return compare(lhs, rhs) !=  0; }
    friend bool operator== (const StringRefBase& lhs, const StringRefBase& rhs) { return compare(lhs, rhs) ==  0; }

  private:
    const Char* m_str = nullptr;
    size_t m_length   = 0;
  };

  using StringRef = StringRefBase<char>;

}

#pragma once

#include <librt/Algorithm.hpp>

namespace rt
{
  template<typename To, typename From>
  inline To bitCast(const From& from) requires(sizeof(To) == sizeof(From))
  {
    To to;
    copy(reinterpret_cast<const char*>(&from), reinterpret_cast<const char*>(&from)+sizeof from, reinterpret_cast<char*>(&to));
    return to;
  }

  namespace details
  {
    template<typename From>
    struct AutoBitCastResult
    {
    public:
      AutoBitCastResult(const From& from) : m_from(from) {}

    public:
      template<typename To>
      operator To() const { return bitCast<To>(m_from); }

    private:
      const From& m_from;
    };
  }

  template<typename From>
  inline auto autoBitCast(const From& from) { return details::AutoBitCastResult<From>(from); }
}

#pragma once

#include <generic/Error.hpp>
#include <generic/Types.hpp>

#include <type_traits>
#include <limits>

#include <stddef.h>
#include <stdint.h>

namespace core::vfs
{
  typedef uword_t size_t;
  typedef word_t ssize_t;
  typedef word_t off_t;

  typedef uword_t dev_t;
  typedef uword_t ino_t;

  typedef word_t result_t;
  typedef word_t fd_t;


  static_assert(std::is_same_v<size_t, ::size_t>);

  static constexpr dev_t UNKNWON_DEV = std::numeric_limits<dev_t>::max();
  static constexpr ino_t UNKNWON_INO = std::numeric_limits<ino_t>::max();

  enum class Type { REGULAR_FILE, DIRECTORY, SYMBOLIC_LINK, OTHER };
  enum class Anchor { BEGIN, CURRENT , END };

  inline Result<Type> typeFromUser(uword_t type)
  {
    switch(static_cast<Type>(type))
    {
    case Type::REGULAR_FILE:
    case Type::DIRECTORY:
    case Type::SYMBOLIC_LINK:
    case Type::OTHER:
      return static_cast<Type>(type);
    default:
      return ErrorCode::INVALID;
    }
  }

  inline Result<Anchor> anchorFromUser(uword_t type)
  {
    switch(static_cast<Anchor>(type))
    {
    case Anchor::BEGIN:
    case Anchor::CURRENT:
    case Anchor::END:
      return static_cast<Anchor>(type);
    default:
      return ErrorCode::INVALID;
    }
  }
}


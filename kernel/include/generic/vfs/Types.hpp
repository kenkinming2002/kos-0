#pragma once

#include <stddef.h>
#include <stdint.h>

#include <type_traits>
#include <limits>

namespace core::vfs
{
  typedef size_t addr_t;
  typedef uint32_t dev_t;
  typedef uint32_t ino_t;

  static constexpr dev_t UNKNWON_DEV = std::numeric_limits<dev_t>::max();
  static constexpr ino_t UNKNWON_INO = std::numeric_limits<ino_t>::max();

  enum class Type      { REGULAR_FILE, DIRECTORY, SYMBOLIC_LINK, OTHER };
}


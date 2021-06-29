#pragma once

#include <stdint.h>

#include <type_traits>
#include <limits>

using word_t  = intptr_t;
using uword_t = uintptr_t;

using size_t  = uword_t;
using ssize_t = word_t;

using off_t = word_t;

using dev_t = word_t;
using ino_t = word_t;

using result_t = word_t;
using fd_t     = word_t;

using status_t = word_t;
using pid_t    = word_t;

static constexpr fd_t FD_NONE = -1;
static constexpr dev_t UNKNWON_DEV = -1;
static constexpr ino_t  UNKNWON_INO = -1;

enum class Type   : uword_t { REGULAR_FILE, DIRECTORY, SYMBOLIC_LINK, OTHER };
enum class Anchor : uword_t { BEGIN, CURRENT , END };

enum class Prot : uword_t
{
  NONE  = 0,
  READ  = 1u << 0,
  WRITE = 1u << 1,
  EXEC  = 1u << 2
};


inline constexpr Prot operator|(Prot lhs, Prot rhs) { return static_cast<Prot>(static_cast<std::underlying_type_t<Prot>>(lhs) | static_cast<std::underlying_type_t<Prot>>(rhs)); }
inline constexpr Prot operator&(Prot lhs, Prot rhs) { return static_cast<Prot>(static_cast<std::underlying_type_t<Prot>>(lhs) & static_cast<std::underlying_type_t<Prot>>(rhs)); }

inline constexpr Prot& operator|=(Prot& lhs, Prot rhs) { lhs = lhs | rhs; return lhs; }
inline constexpr Prot& operator&=(Prot& lhs, Prot rhs) { lhs = lhs & rhs; return lhs; }


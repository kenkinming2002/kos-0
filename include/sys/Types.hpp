#pragma once

#include <stdint.h>
#include <stddef.h>

#include <type_traits>
#include <limits>

using word_t  = intptr_t;
using uword_t = uintptr_t;

using size_t  = uword_t;
using ssize_t = word_t;

using off_t = word_t;

using device_t = uword_t;
using inode_t = uword_t;

using result_t = word_t;
using fd_t     = word_t;

static constexpr device_t UNKNWON_DEV = std::numeric_limits<device_t>::max();
static constexpr inode_t  UNKNWON_INO = std::numeric_limits<inode_t>::max();

enum class Type   : uword_t { REGULAR_FILE, DIRECTORY, SYMBOLIC_LINK, OTHER };
enum class Anchor : uword_t { BEGIN, CURRENT , END };


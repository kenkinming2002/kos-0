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

enum Prot
{
  PROT_NONE  = 0,
  PROT_READ  = 1u << 0,
  PROT_WRITE = 1u << 1,
  PROT_EXEC  = 1u << 2
};


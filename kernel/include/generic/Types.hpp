#pragma once

#include <stdint.h>

namespace core
{
  typedef intptr_t  word_t;
  typedef uintptr_t uword_t;

  static_assert(sizeof(word_t) == sizeof(void*));
  static_assert(sizeof(uword_t) == sizeof(void*));
}

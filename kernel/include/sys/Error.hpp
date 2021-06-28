#pragma once

#include <sys/Types.hpp>

enum class ErrorCode : uword_t
{
  INVALID=1,
  EXIST,
  NOT_EXIST,
  BUSY,
  OUT_OF_MEMORY,
  UNSUPPORTED,
  NFILE,
  BADFD,
  FAULT,
  OVERFLOW,
  OTHER
};



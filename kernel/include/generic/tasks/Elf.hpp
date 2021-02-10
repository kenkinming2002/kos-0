#pragma once

#include <i686/tasks/Task.hpp>

#include <librt/Optional.hpp>

#include <stddef.h>

namespace core::tasks
{
  int loadElf(Task& task, char* data, size_t length);
}


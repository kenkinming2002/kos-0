#pragma once

#include <i686/tasks/Task.hpp>

#include <optional>
#include <stddef.h>

namespace core::tasks
{
  int loadElf(Task& task, char* data, size_t length);
}


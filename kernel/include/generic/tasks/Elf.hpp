#pragma once

#include <i686/tasks/Task.hpp>

#include <optional>
#include <stddef.h>

namespace core::tasks
{
  std::optional<Task> loadElf(char* data, size_t length);
}


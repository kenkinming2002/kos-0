#pragma once

#include "librt/SharedPtr.hpp"
#include <i686/tasks/Task.hpp>

#include <librt/Optional.hpp>

#include <stddef.h>

namespace core::tasks
{
  Result<void> loadElf(rt::SharedPtr<Task> task, rt::SharedPtr<vfs::File> file);
}


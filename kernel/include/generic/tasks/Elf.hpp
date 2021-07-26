#pragma once

#include <generic/vfs/File.hpp>
#include <i686/tasks/Task.hpp>

#include <librt/Result.hpp>

namespace core::tasks
{
  Result<void> loadElf(rt::SharedPtr<Task> task, rt::SharedPtr<vfs::File> file);
}


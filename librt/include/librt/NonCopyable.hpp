#pragma once

namespace rt
{
  struct NonCopyable
  {
    NonCopyable() = default;

    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
  };
}

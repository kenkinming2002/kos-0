#include <librt/Hooks.hpp>

namespace rt::hooks
{
  [[gnu::weak]] bool validAddress(void* ptr) { return ptr != nullptr; }
}

#include <i686/internals/Internals.hpp>

#include <i686/internals/Segmentation.hpp>

namespace core::internals
{
  void initialize()
  {
    initializeSegmentation();
  }
}

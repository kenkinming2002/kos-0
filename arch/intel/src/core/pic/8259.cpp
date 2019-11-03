#include <intel/core/pic/8259.hpp>

namespace core::pic
{
  Controller8259::Controller8259(uint8_t masterOffset, uint8_t slaveOffset)
  {
    this->remap(masterOffset, slaveOffset);
  }

  void Controller8259::remap(uint8_t masterOffset, uint8_t slaveOffset) const
  {
  }

  void Controller8259::acknowledge(uint8_t interrupt) const
  {
  }
}

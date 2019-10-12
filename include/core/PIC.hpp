#pragma once

#include <stdint.h>

class PICController
{
public:
  PICController();

public:
  int init() const;

public:
  int remap(uint8_t masterOffset, uint8_t slaveOffset) const;
  int acknowledge(uint32_t interrupt) const;
};

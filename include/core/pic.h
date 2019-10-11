#pragma once

#include <stdint.h>

int pic_init();
int pic_acknowledge(uint32_t interrupt);

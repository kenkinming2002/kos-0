#pragma once

#include <stdint.h>

extern "C" void core_tasks_switch_esp(uintptr_t* espPrevious, uintptr_t* espNext);


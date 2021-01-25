#pragma once

/* Note: Lower numbers indicate higher priority */
#define INIT_NOLOG  [[gnu::init_priority(101)]]
#define INIT_EARLY  [[gnu::init_priority(102)]]
#define INIT_NORMAL [[gnu::init_priority(103)]]
#define INIT_LATE   [[gnu::init_priority(104)]]

#define INIT_FUNCTION_NOLOG  [[gnu::constructor(101)]]
#define INIT_FUNCTION_EARLY  [[gnu::constructor(102)]]
#define INIT_FUNCTION_NORMAL [[gnu::constructor(103)]]
#define INIT_FUNCTION_LATE   [[gnu::constructor(104)]]

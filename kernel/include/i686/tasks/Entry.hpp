#pragma once

#include <i686/tasks/Registers.hpp>

extern "C" [[noreturn]] void core_tasks_entry(core::tasks::Registers* registers);

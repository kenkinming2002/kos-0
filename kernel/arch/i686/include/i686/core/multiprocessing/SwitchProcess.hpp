#pragma once

#include <generic/core/Process.hpp>

extern "C" void switch_process(core::Process* previousProcess, core::Process* nextProcess);

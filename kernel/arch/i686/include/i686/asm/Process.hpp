#pragma once

#include <i686/core/Syscall.hpp>

extern "C" [[noreturn]] void enter_user_mode(core::State state);

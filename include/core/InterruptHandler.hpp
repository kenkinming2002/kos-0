#pragma once

#include <stdint.h>
#include <core/Interrupt.hpp>
#include <core/InterruptHandlerWrapper.hpp>

int initIDT(IDTEntry idtEntries[256]);

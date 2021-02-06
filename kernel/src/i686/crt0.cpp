#include <generic/devices/Framebuffer.hpp>
#include <generic/tasks/Scheduler.hpp>
#include <generic/memory/Memory.hpp>
#include <generic/BootInformation.hpp>

#include <i686/memory/MemoryMapping.hpp>
#include <i686/internals/Internals.hpp>
#include <i686/interrupts/Interrupts.hpp>

void kmain();

extern "C" void _start(BootInformation* bootInformation)
{
  ::bootInformation = bootInformation;

  core::devices::Framebuffer::initialize();
  core::internals::initialize();
  core::interrupts::initialize();
  core::memory::MemoryMapping::initialize();
  core::memory::initialize();
  core::tasks::Scheduler::initialize();

  kmain();
}


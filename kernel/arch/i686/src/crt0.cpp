#include <generic/icxxabi.hpp>

#include <i686/core/Interrupt.hpp>
#include <intel/core/pic/8259.hpp>
#include <intel/core/Exceptions.hpp>
#include <i686/core/Syscall.hpp>
#include <generic/core/Memory.hpp>
#include <i686/core/multiprocessing/Yield.hpp>

#include <generic/core/IPC.hpp>

extern "C" void _init();
extern "C" void _fini();

extern "C" void kmain();

extern "C" void _start()
{
  core::segmentation::init();
  core::interrupt::init();
  core::pic::controller8259::init();
  core::exceptions::init();
  core::init_syscall();
  core::memory::init();
  core::multiprocessing::init();
  core::ipc::init();

  _init();

  kmain();
  __cxa_finalize(nullptr);
  _fini();
}

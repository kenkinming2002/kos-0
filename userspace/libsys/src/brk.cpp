#include <libsys/brk.hpp>

#include <libsys/Syscalls.hpp>

extern "C" char _end[];

namespace
{
  constexpr char* programEnd = _end;
  char* programBreak = _end;
}

int brk(void* addr)
{
  char* newProgramBreak = static_cast<char*>(addr);

  if(newProgramBreak == programBreak)
    return 0;

  if(newProgramBreak < programEnd)
    return -1;

  // TODO: Possibly implement mremap
  if(programEnd == programBreak)
  {
    if(sys_mmap(reinterpret_cast<uintptr_t>(programEnd), newProgramBreak - programEnd, PROT_READ | PROT_WRITE, FD_NONE, 0) != 0)
      return -1;
  }
  else if(programEnd == newProgramBreak)
  {
    if(sys_munmap(reinterpret_cast<uintptr_t>(programEnd), programBreak - programEnd) != 0)
      return -1;
  }
  else
  {
    if(sys_mremap(reinterpret_cast<uintptr_t>(programEnd), programBreak - programEnd, newProgramBreak - programEnd) != 0)
      return -1;
  }

  programBreak = newProgramBreak;
  return 0;
}

void* sbrk(intptr_t increment)
{
  char* oldProgramBreak = programBreak;
  if(brk(programBreak+increment) != 0)
    return reinterpret_cast<void*>(-1);

  return oldProgramBreak;
}


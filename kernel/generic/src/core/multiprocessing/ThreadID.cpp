#include <generic/core/multiprocessing/ThreadID.hpp>

#include <bitset>

namespace core::multiprocessing
{

  //TODO: Change uint32_t to integer with native machine word size
  namespace
  {
    std::bitset<MAX_PROCESS_COUNT> tidBitset;
  }

  ThreadID allocThreadID()
  {
    for(size_t i=0; i<MAX_PROCESS_COUNT; ++i)
      if(!tidBitset[i])
      {
        tidBitset[i] = 1;
        return i;
      }

    return -1;
  }

  void freeThreadID(ThreadID tid)
  {
    // TODO: maybe optimize this branch with builtin_unlikely
    if(tid<0 || tid == anyThreadID) [[unlikely]]
      return; 

    tidBitset[tid] = 0;
  }
}

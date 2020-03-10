#include <i686/core/MultiProcessing.hpp>

#include <algorithm>

namespace core::multiprocessing
{
  boost::intrusive::slist<core::Process, member_hook> processesList;

  core::Process* findProcess(ThreadID tid)
  {
    auto it = std::find_if(processesList.begin(), processesList.end(), [&](auto& process){
        return process.tid == tid;
    });

    if(it==processesList.end())
      return nullptr;
    else
      return &(*it);
  }
}

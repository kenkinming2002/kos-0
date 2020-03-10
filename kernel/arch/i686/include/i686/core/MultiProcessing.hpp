#pragma once

#include <generic/core/multiprocessing/ThreadID.hpp>
#include <generic/core/Process.hpp>

#include <boost/intrusive/slist.hpp>

namespace core::multiprocessing
{
  using member_hook = boost::intrusive::member_hook<core::Process, 
        boost::intrusive::slist_member_hook<>, &core::Process::slist_hook>;
  extern boost::intrusive::slist<core::Process, member_hook> processesList;

  /**
   * Find of process of specific Thread ID
   *
   * @param tid Thread ID of process
   * @return pointer to found process, nullptr if not found
   **/
  core::Process* findProcess(ThreadID tid);
}

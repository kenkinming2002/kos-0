#pragma once

#include <generic/core/Process.hpp>

#include <boost/intrusive/slist.hpp>

namespace core::multiprocessing
{
  using member_hook = boost::intrusive::member_hook<core::Process, 
        boost::intrusive::slist_member_hook<>, &core::Process::slist_hook>;
  extern boost::intrusive::slist<core::Process, member_hook> processesList;
}

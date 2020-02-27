#pragma once

#include <generic/core/Process.hpp>

#include <boost/intrusive/slist.hpp>

namespace core::multiprocessing
{
  extern boost::intrusive::slist<core::Process> processesList;
}

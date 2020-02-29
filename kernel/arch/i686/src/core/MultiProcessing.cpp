#include <i686/core/MultiProcessing.hpp>

namespace core::multiprocessing
{
  boost::intrusive::slist<core::Process, member_hook> processesList;
}

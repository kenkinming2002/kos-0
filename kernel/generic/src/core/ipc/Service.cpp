#include <generic/core/ipc/Service.hpp>

#include <i686/core/MultiProcessing.hpp>

#include <string.h>

namespace core::ipc
{
  namespace
  {
    /*
     * Same as posix strdup, but use new to allocate memory instead of malloc
     * TODO: impose and upper limit on string length
     */
    inline std::unique_ptr<const char[]> strdup(const char* str)
    {
      size_t len = strlen(str);

      char* copy = new char[len+1];
      if(!copy)
        return nullptr;

      memcpy(copy, str, len);
      copy[len] = '\0';

      return std::unique_ptr<const char[]>(copy);
    }
  }

  Service::Service(const char* name, core::Process& owner) : m_name(strdup(name)), m_owner(&owner) {}

  bool Service::hasName(const char* name) const { return (*this) && strcmp(m_name.get(), name) == 0; }

  namespace
  {
    constexpr static size_t MAX_SERVICE_COUNT = 64;
    Service services[MAX_SERVICE_COUNT];
  }

  handle_t service_create(const char* name)
  {
    //TODO: consider checking for duplicates
    for(size_t i=0; i<MAX_SERVICE_COUNT; ++i)
      if(!services[i])
      {
        services[i] = Service(name, core::multiprocessing::processesList.front());
        return i;
      }

    return -1;
  }

  int service_destroy(handle_t handle)
  {
    // Find the service to destroy
    // TODO  check the handle
    // TODO: Check permission only creator of service can destroy it
    services[handle] = Service();
    return 0;
  }

  multiprocessing::ThreadID service_locate(const char* name)
  {
    // Find the target service
    for(size_t i=0; i<MAX_SERVICE_COUNT; ++i)
      if(services[i].hasName(name))
      {
        auto& service = services[i];
        return service.owner().tid;
      }

    return -1;
  }
}

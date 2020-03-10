#pragma once

#include <generic/core/Process.hpp>
#include <generic/core/multiprocessing/ThreadID.hpp>

#include <memory>

namespace core::ipc
{
  using handle_t = int;
  struct Service
  {
  public:
    constexpr Service() = default;
    /**
     * Simple Contructor
     *
     * @param name Name of the service, string is duplicated
     * @param owner Owner of the service
     **/
    Service(const char* name, core::Process& owner);

  public:
    /** 
     * @return If service is present
     **/
    constexpr operator bool() const { return m_owner && m_name; }
    /** 
     * Simple getter
     * @return Owner of the service
     **/
    constexpr core::Process& owner() const { return *m_owner; }

  public:
    /**
     * @param name name of service
     * @return If the name of service is @p name
     **/
    bool hasName(const char* name) const;

  private:
    std::unique_ptr<const char[]> m_name = nullptr;
    core::Process* m_owner = nullptr;
  };


  /**
   * @brief Create a service, requires elevated privilleges.
   *
   * @param name name of the service
   * @return handle to the created service, negative value on error
   **/
  handle_t service_create(const char* name);

  /**
   * @brief Destroy service created, requires elevated privilleges and only
   *        owner of the service can destroy the service.
   *
   * @param handle handle to the service to destroy
   * @return 0 on success, any other value indicates an error
   **/
  int service_destroy(handle_t handle);

  /**
   * @brief Locate service with given name
   *
   * @param name name of the service to connect to
   * @return thread ID of process providing the service, negative value on error
   **/
  multiprocessing::ThreadID service_locate(const char* name);
}

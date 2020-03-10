#include <generic/core/IPC.hpp>

#include <i686/core/Syscall.hpp>

#include <generic/core/ipc/Service.hpp>
#include <generic/core/ipc/Message.hpp>

/** @file 
 *  
 *  Syscall bindings for IPC
 **/

namespace core::ipc
{
  /** @namespace core::ipc
   *
   * Interprocess-Communication API
   *
   * The following 2 functions are used to register and deregister service
   * providers.
   *  - create_service(name) -> service_handle
   *  - locate_service(name) -> threaadID
   *  - destroy_service(service_handle) -> void
   *
   *  With a conncetion handle, interprocee-communication can proceed via the
   *  following 2 functions
   *    - send(dst, msg) -> void
   *    - recv(src)      -> msg
   **/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  int syscall_service_create(const core::State state)
  {
    auto name = reinterpret_cast<const char*>(state.a1);
    return service_create(name);
  }

  int syscall_service_destroy(const core::State state)
  {
    auto handle = state.a1_s;
    return service_destroy(handle);
  }

  int syscall_service_locate(const core::State state)
  {
    auto name = reinterpret_cast<const char*>(state.a1);
    return service_locate(name);
  }

  int syscall_message_send(const core::State state)
  {
    multiprocessing::ThreadID dst = state.a1_s;
    auto buf = reinterpret_cast<const char*>(state.a2);
    auto len = state.a3;
    return message_send(dst, buf, len);
  }

  int syscall_message_recv(const core::State state)
  {
    auto src = state.a1_s;
    auto buf = reinterpret_cast<char*>(state.a2);
    auto len = state.a3;
    return message_recv(src, buf, len);
  }
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


  /** @fn void init()
   *
   *  @brief Initialize interprocess communication system
   *
   *  Register all system calls for interprocess commmunincation
   **/
  void init()
  {
    core::register_syscall_handler(SYSCALL_SERVICE_CREATE,  &syscall_service_create);
    core::register_syscall_handler(SYSCALL_SERVICE_DESTROY, &syscall_service_destroy);
    core::register_syscall_handler(SYSCALL_SERVICE_LOCATE, &syscall_service_locate);

    core::register_syscall_handler(SYSCALL_MESSAGE_SEND,  &syscall_message_send);
    core::register_syscall_handler(SYSCALL_MESSAGE_RECV,  &syscall_message_recv);
  }
}

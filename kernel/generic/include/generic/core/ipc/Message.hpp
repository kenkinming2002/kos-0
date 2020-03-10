#pragma once

#include <generic/core/multiprocessing/ThreadID.hpp>

#include <boost/intrusive/slist.hpp>
#include <optional>

#include <memory>


namespace core
{
  class Process;
}


namespace core::ipc
{
  // NOTE: Perhaps good idea to consider using deque to reduce memory
  //       footprint
  struct Message : public boost::intrusive::slist_base_hook<>
  {
  public:
    Message() = default;

  public:
    /**
     * @return If message is valid/non-empty
     **/
    operator bool() const;
    multiprocessing::ThreadID src() const { return m_src; }

  public:
    /**
     * Contrust a message, making a deep copy
     *
     * @param msg message
     * @param len length of message
     **/
    Message(const char* msg, size_t len, multiprocessing::ThreadID src);
    /**
     * Copy message to given buffer
     *
     * @param buf buffer
     * @param len length of buffer
     *
     * @return number of byte copied, -1 on error
     **/
    int copyTo(char* buf, size_t len);


  public:
    /** Send message to given process. This message is moved from and no longer
     *  valid
     *
     *  @param process message to send message to
     *  @return 0 on sucess, any other values indicate an error
     **/
    int sendTo(core::Process& process);

    /** 
     * Receive message that are sent to process
     **/
    int recvFrom(core::Process& process, multiprocessing::ThreadID src);

  private:
    //const char* m_msg = nullptr;
    std::unique_ptr<const char[]> m_msg;
    size_t m_len = 0u;
    multiprocessing::ThreadID m_src = -1;
  };


  /**
   * @brief Send a message
   *
   * @param dst thread ID of destination process
   * @param buf buffer containing message to send
   * @param len length of message
   *
   * @return destination of message, negative value on error
   **/
  int message_send(multiprocessing::ThreadID dst, const char* buf, size_t len);

  /**
   * @brief Receive a message
   *
   * @param dest thread ID of source process
   * @param buf buffer to store the received message
   * @param len size of buffer
   *
   * @return source of message, negative value on error
   **/
  int message_recv(multiprocessing::ThreadID src, char* buf, size_t len);
}

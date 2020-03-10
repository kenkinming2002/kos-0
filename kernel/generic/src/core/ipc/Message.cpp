#include <generic/core/ipc/Message.hpp>

#include <algorithm>
#include <string.h>

#include <generic/core/Process.hpp>
#include <i686/core/MultiProcessing.hpp>

#include <utility>

namespace core::ipc
{
  Message::operator bool() const
  {
    return static_cast<bool>(m_msg);
  }

  namespace
  {
    inline std::unique_ptr<const char[]> dup(const char* str, size_t len)
    {
      char* copy = new char[len];
      if(!copy)
        return nullptr;

      memcpy(copy, str, len);
      return std::unique_ptr<const char[]>(copy);
    }
  }

  Message::Message(const char* msg, size_t len, multiprocessing::ThreadID src) : m_msg(dup(msg, len)), m_len(len), m_src(src) {}

  int Message::copyTo(char* buf, size_t len)
  {
    if(m_len>len)
      return -1; // Insufficient buffer size

    memcpy(buf, m_msg.get(), m_len);
    return 0;
  }

  int Message::sendTo(core::Process& process)
  {
    auto message = new Message();
    if(!message)
      return -1;
    *message = std::move(*this);

    process.ipcMessages.push_back(*message);
    return 0;
  }

  int Message::recvFrom(core::Process& process, multiprocessing::ThreadID src)
  {
    switch(src)
    {
    case multiprocessing::anyThreadID:
      if(process.ipcMessages.empty())
        return -1;

      *this = std::move(process.ipcMessages.front());
      process.ipcMessages.pop_front_and_dispose([](auto* message){ delete message; });
      return 0;
    default:
      auto before = std::adjacent_find(process.ipcMessages.begin(), process.ipcMessages.end(), [&](auto&, auto& message) -> bool {
          return message.src() == src;
      });
      if(before == process.ipcMessages.end())
        return -1;

      *this = std::move(*std::next(before));
      process.ipcMessages.erase_after_and_dispose(before, [](auto* message){ delete message; });
      return 0;
    }
  }


  int message_send(multiprocessing::ThreadID dst, const char* buf, size_t len)
  {
    auto& srcProcess = core::multiprocessing::processesList.front();
    auto* dstProcess = core::multiprocessing::findProcess(dst);
    if(!dstProcess)
      return -1;

    auto message =  Message(buf, len, srcProcess.tid);
    if(!message)
      return -1;

    message.sendTo(*dstProcess);

    return dst;
  }

  int message_recv(multiprocessing::ThreadID src, char* buf, size_t len)
  {
      auto& dstProcess = core::multiprocessing::processesList.front();

      Message message;
      message.recvFrom(dstProcess, src);
      if(!message)
        return -1;

      src = message.src();
      int status;
      if((status = message.copyTo(buf, len)) != 0)
        return status;

      return src;
  }
}

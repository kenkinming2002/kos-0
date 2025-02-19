#include <generic/vfs/FileDescriptors.hpp>

namespace core::vfs
{
  template<typename T>
  auto asUnsigned(T t) { return std::make_unsigned_t<T>(t); }

  template<typename T>
  auto asSigned(T t) { return std::make_signed<T>(t); }

  rt::SharedPtr<FileDescriptors> FileDescriptors::clone()
  {
    auto result = rt::makeShared<FileDescriptors>();
    if(!result)
      return nullptr;

    for(size_t i=0; i<MAX_FD; ++i)
      if(result->m_files[i])
        result->m_files[i] = m_files[i]->clone();

    return result;
  }

  Result<fd_t> FileDescriptors::addFile(rt::SharedPtr<File> file)
  {
    ASSERT(file);

    core::LockGuard guard(m_lock);
    for(size_t fd=0; fd<MAX_FD; ++fd)
      if(!m_files[fd])
      {
        m_files[fd] = rt::move(file);
        return fd;
      }

    return ErrorCode::NFILE;
  }

  Result<rt::SharedPtr<File>> FileDescriptors::getFile(fd_t fd)
  {
    if(fd<0 || asUnsigned(fd)>=MAX_FD)
      return ErrorCode::BADFD;

    core::LockGuard guard(m_lock);
    if(!m_files[fd])
      return ErrorCode::BADFD;

    return m_files[fd];
  }

  Result<void> FileDescriptors::removeFile(fd_t fd)
  {
    if(fd<0 || asUnsigned(fd)>=MAX_FD)
      return ErrorCode::BADFD;

    core::LockGuard guard(m_lock);
    if(!m_files[fd])
      return ErrorCode::BADFD;

    m_files[fd].reset();
    return {};
  }
}

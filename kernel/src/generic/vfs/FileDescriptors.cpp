#include <generic/vfs/FileDescriptors.hpp>

namespace core::vfs
{
  template<typename T>
  auto asUnsigned(T t) { return std::make_unsigned_t<T>(t); }

  template<typename T>
  auto asSigned(T t) { return std::make_signed<T>(t); }

  Result<int> FileDescriptors::addFile(vfs::File file)
  {
    ASSERT(file.isOpen());
    for(size_t fd=0; fd<m_files.size(); ++fd)
      if(!m_files[fd].isOpen())
      {
        m_files[fd] = rt::move(file);
        return fd;
      }

    if(m_files.size() == MAX_FD)
    {
      return ErrorCode::NFILE;
    }

    int fd = m_files.size();
    m_files.pushBack(rt::move(file));
    return fd;
  }

  Result<File> FileDescriptors::getFile(int fd)
  {
    if(fd<0 || asUnsigned(fd)>=m_files.size() || !m_files[fd].isOpen())
      return ErrorCode::BADFD;

    return m_files[fd];
  }

  Result<void> FileDescriptors::removeFile(int fd)
  {
    if(fd<0 || asUnsigned(fd)>=m_files.size() || !m_files[fd].isOpen())
      return ErrorCode::BADFD;

    m_files[fd].close();
    return {};
  }
}

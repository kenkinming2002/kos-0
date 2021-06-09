#include <generic/vfs/File.hpp>

namespace core::vfs
{
  void File::close()
  {
    m_vnode.reset();
  }

  Result<File::Stat> File::stat() { return m_vnode->inode().stat(); }

  Result<ssize_t> File::seek(Anchor anchor, off_t offset)
  {
    size_t newPos;
    switch(anchor)
    {
    case Anchor::BEGIN:
      newPos = 0;
      break;
    case Anchor::CURRENT:
      newPos = m_pos;
      break;
    case Anchor::END:
    {
      auto stat = this->stat();
      if(!stat)
        return stat.error();
      newPos = stat->size;
      break;
    }
    default:
      return ErrorCode::INVALID;
    }
    if(__builtin_add_overflow(newPos, offset, &newPos))
      return ErrorCode::OVERFLOW;

    /* Even though it is possible to have a file the size of
     * std::numeric_limits<size_t>::max(), possibly from lazy allocation
     * it is impossible to seek normally on such a file since return value of
     * seek is ssize_t, which has a smaller maximum value
     */
    if(newPos>std::numeric_limits<ssize_t>::max())
      return ErrorCode::INVALID;

    m_pos = newPos;
    return newPos;
  }

  Result<ssize_t> File::read(char* buf, size_t length)
  {
    /* Clamp length, which works since we need to transfer AT MOST length bytes,
     * but not EXACTLY length bytes */
    if(length>std::numeric_limits<ssize_t>::max())
      length = std::numeric_limits<ssize_t>::max();

    if(length>std::numeric_limits<ssize_t>::max())
      return ErrorCode::INVALID;

    auto result = m_vnode->inode().read(buf, length, m_pos);
    if(!result)
      return result.error();

    seek(Anchor::CURRENT, length);
    return *result;
  }

  Result<ssize_t> File::write(const char* buf, size_t length)
  {
    /* Clamp length, which works since we need to transfer AT MOST length bytes,
     * but not EXACTLY length bytes */
    if(length>std::numeric_limits<ssize_t>::max())
      length = std::numeric_limits<ssize_t>::max();

    auto result = m_vnode->inode().write(buf, length, m_pos);
    if(!result)
      return result.error();

    seek(Anchor::CURRENT, length);
    return *result;
  };

  Result<void> File::resize(size_t size)
  {
    return m_vnode->inode().resize(size);
  }

  Result<ssize_t> File::readdir(char* buf, size_t length)
  {
    /* Clamp length, which works since we need to transfer AT MOST length bytes,
     * but not EXACTLY length bytes */
    if(length>std::numeric_limits<ssize_t>::max())
      length = std::numeric_limits<ssize_t>::max();

    return m_vnode->inode().readdir(buf, length);
  }

  Result<void> File::mount(Mountable& mountable, rt::StringRef arg)
  {
    return m_vnode->mount(mountable, arg);
  }

  Result<void> File::umount()
  {
    return m_vnode->umount();
  }

  Result<File> File::lookup(rt::StringRef name)
  {
    auto vnode = m_vnode->lookup(name);
    if(!vnode)
      return vnode.error();

    return File(rt::move(*vnode));
  }

  Result<File> File::create(rt::StringRef name, Type type)
  {
    auto vnode = m_vnode->create(name, type);
    if(!vnode)
      return vnode.error();

    return File(rt::move(*vnode));
  }

  Result<void> File::link(rt::StringRef name, Inode& inode)
  {
    return m_vnode->link(name, inode);
  }

  Result<void> File::unlink(rt::StringRef name)
  {
    return m_vnode->unlink(name);
  }
}

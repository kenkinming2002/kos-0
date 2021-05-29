#include <generic/vfs/File.hpp>

namespace core::vfs
{
  File::File(rt::SharedPtr<Vnode> vnode) : m_vnode(rt::move(vnode))
  {
    ASSERT(m_vnode);
    ASSERT(!m_vnode->negative());
  }

  Result<File::Stat> File::stat() { return m_vnode->inode().stat(); }
  Result<size_t> File::read(char* buf, size_t length, addr_t off)        { return m_vnode->inode().read(buf, length, off); }
  Result<size_t> File::write(const char* buf, size_t length, addr_t off) { return m_vnode->inode().write(buf, length, off); };
  Result<void>   File::resize(size_t size)                               { return m_vnode->inode().resize(size); }

  Result<void> File::iterate(iterate_callback_t cb, void* data) { return m_vnode->inode().iterate(cb, data); }

  Result<void> File::mount(Mountable& mountable, rt::Span<rt::StringRef> args)
  {
    return m_vnode->mount(mountable, args);
  }

  Result<void> File::umount()
  {
    return m_vnode->umount();
  }

  Result<File> File::lookup(rt::StringRef name)
  {
    auto vnode = m_vnode->lookup(name);
    if(!vnode)
      return ErrorCode::NOT_EXIST;

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

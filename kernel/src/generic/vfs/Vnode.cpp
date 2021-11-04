#include <generic/vfs/Vnode.hpp>

#include <generic/vfs/Mountable.hpp>

#include <librt/SharedPtr.hpp>

namespace core::vfs
{
  rt::SharedPtr<Inode> Vnode::inode()
  {
    core::LockGuard guard(m_lock);
    return _inode();
  }

  rt::SharedPtr<Inode>& Vnode::_inode()
  {
    return m_mountedInode ? m_mountedInode : m_inode;
  }

  rt::SharedPtr<Vnode> Vnode::lookup(rt::StringRef name)
  {
    core::LockGuard guard(m_lock);
    auto it = m_childs.find(name);
    if(it != m_childs.end())
      return it->second;

    auto childInode = _inode()->lookup(name);
    if(!childInode)
      return nullptr;

    auto childVnode = rt::makeShared<Vnode>(this, rt::move(childInode));
    it = m_childs.insert({rt::String(name), rt::move(childVnode)});
    return it->second;
  }

  Result<void> Vnode::mount(Mountable& mountable, rt::StringRef arg)
  {
    core::LockGuard guard(m_lock);
    if(m_mountedInode)
      return ErrorCode::EXIST;

    auto mountedInode = mountable.mount(arg);
    if(!mountedInode)
      return mountedInode.error();

    m_childs = {}; // Clear the cache
    m_mountedInode = rt::move(*mountedInode);
    return {};
  }

  Result<void> Vnode::umount()
  {
    core::LockGuard guard(m_lock);
    if(!m_mountedInode)
      return ErrorCode::NOT_EXIST;

    m_childs = {}; // Clear the cache
    m_mountedInode = nullptr;
    return {};
  }

  Result<rt::SharedPtr<Vnode>> Vnode::create(rt::StringRef name, Type type)
  {
    core::LockGuard guard(m_lock);
    auto it = m_childs.find(name);
    if(it != m_childs.end())
      return ErrorCode::EXIST;

    auto childInode = _inode()->create(name ,type);
    if(!childInode)
      return childInode.error();

    auto childVnode = rt::makeShared<Vnode>(this, rt::move(*childInode));
    it = m_childs.insert({rt::String(name), rt::move(childVnode)});
    return it->second;
  }

  Result<void> Vnode::link(rt::StringRef name, Inode& inode)
  {
    /* We need a shared pointer implementation to properly implement link */
    ASSERT_UNIMPLEMENTED;
    return ErrorCode::UNSUPPORTED;
  }

  Result<void> Vnode::unlink(rt::StringRef name)
  {
    core::LockGuard guard(m_lock);
    auto it = m_childs.find(name);
    if(it == m_childs.end())
      return ErrorCode::NOT_EXIST;

    auto result = _inode()->unlink(name); /* We have to think about the problem with orphaned inode */
    if(!result)
      return result.error();

    m_childs.erase(it);
    return {};
  }
}

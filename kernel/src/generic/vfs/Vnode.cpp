#include <generic/vfs/Vnode.hpp>
#include <generic/vfs/Path.hpp>

#include <librt/SharedPtr.hpp>
#include <librt/UniquePtr.hpp>
#include <librt/StringRef.hpp>

namespace core::vfs
{
  /* Precondition:  !this->negative()
   * Postcondition: result.m_state == State::NORMAL || result.m_state == State::MOUNTED*/
  rt::SharedPtr<Vnode>& Vnode::lookup_ref(rt::StringRef name)
  {
    auto it = m_childs.find(name);
    if(it == m_childs.end())
      it = m_childs.insert({name, rt::makeShared<Vnode>(this)});

    auto& childVnode = it->second;
    switch(childVnode->m_state)
    {
    case State::NEW:
    case State::UMOUNTED:
    {
      auto childInode = m_inode->lookup(name);
      childVnode->associate(childInode ? rt::move(*childInode) : nullptr);
      break;
    }
    case State::NORMAL:
    case State::MOUNTED:
      break;
    }

    return childVnode;
  }

  Result<rt::SharedPtr<Vnode>> Vnode::lookup(rt::StringRef name)
  {
    auto& vnode = lookup_ref(name);
    if(vnode->negative())
      return ErrorCode::NOT_EXIST;

    ASSERT(vnode.get() != this);
    ASSERT(&vnode->inode() != m_inode.get());

    return vnode;
  }

  Result<void> Vnode::mount(Mountable& mountable, rt::StringRef arg)
  {
    if(m_state == State::NORMAL && !m_inode)
      return ErrorCode::NOT_EXIST;

    if(m_state == State::MOUNTED)
      return ErrorCode::EXIST;

    auto inode = mountable.mount(arg);
    if(!inode)
      return inode.error();

    m_state = State::MOUNTED;
    m_inode = rt::move(*inode);
    return {};
  }

  Result<void> Vnode::umount()
  {
    if(m_state == State::NORMAL && !m_inode)
      return ErrorCode::NOT_EXIST;

    if(m_state == State::MOUNTED)
      return ErrorCode::EXIST;

    /* TODO: We would have to clear our cache */
    m_state = State::UMOUNTED;
    m_inode = nullptr;
    return {};
  }

  Result<rt::SharedPtr<Vnode>> Vnode::create(rt::StringRef name, Type type)
  {
    auto& childVnode = lookup_ref(name);
    if(childVnode->m_state == State::MOUNTED)
      return ErrorCode::BUSY;

    if(!childVnode->negative())
      return ErrorCode::EXIST;

    auto childInode = m_inode->create(name ,type);
    if(!childInode)
      return childInode.error();

    childVnode = rt::makeShared<Vnode>(this);
    childVnode->associate(rt::move(*childInode));
    return childVnode;
  }

  Result<void> Vnode::link(rt::StringRef name, Inode& inode)
  {
    /* We need a shared pointer implementation to properly implement link */
    return ErrorCode::UNSUPPORTED;
  }

  Result<void> Vnode::unlink(rt::StringRef name)
  {
    auto& childVnode = lookup_ref(name);
    if(childVnode->m_state == State::MOUNTED)
      return ErrorCode::BUSY;

    if(childVnode->negative())
      return ErrorCode::NOT_EXIST;

    auto result = m_inode->unlink(name); /* We have to think about the problem with orphaned inode */
    if(!result)
      return result.error();

    // If there is no more reference to the vnode, the associating inode would be freed automatically
    childVnode = rt::makeShared<Vnode>(this);
    childVnode->associate(nullptr);
    return {};
  }
}

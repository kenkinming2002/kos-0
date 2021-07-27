#pragma once

#include <generic/vfs/Inode.hpp>
#include <generic/Error.hpp>

#include <librt/containers/Map.hpp>

#include <librt/String.hpp>
#include <librt/SharedPtr.hpp>
#include <librt/Assert.hpp>
#include <librt/SpinLock.hpp>

namespace core::vfs
{
  class Mountable;
  class Vnode : public rt::SharedPtrHook
  {
  public:
    Vnode(Vnode* parent, rt::SharedPtr<Inode> inode) : m_parent(parent), m_inode(rt::move(inode)) {}

  public:
    const SuperBlock& superBlock() const { return m_inode->superBlock(); }
    SuperBlock& superBlock()             { return m_inode->superBlock(); }

  public:
    rt::SharedPtr<Inode> inode();

  private:
    rt::SharedPtr<Inode>& _inode();

  public:
    Result<void> mount(Mountable& mountable, rt::StringRef arg);
    Result<void> umount();

  public:
    rt::SharedPtr<Vnode> lookup(rt::StringRef name);
    Result<rt::SharedPtr<Vnode>> create(rt::StringRef name, Type type);
    Result<void> link(rt::StringRef name, Inode& inode);
    Result<void> unlink(rt::StringRef name);

  private:
    rt::SpinLock m_lock;

  private:
    Vnode* m_parent; // To support walking .. directories, potential race
    rt::containers::Map<rt::String, rt::SharedPtr<Vnode>> m_childs;

  private:
    rt::SharedPtr<Inode> m_inode;
    rt::SharedPtr<Inode> m_mountedInode;
  };
}

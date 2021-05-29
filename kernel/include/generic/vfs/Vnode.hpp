#pragma once

#include <generic/vfs/Inode.hpp>
#include <generic/vfs/Mountable.hpp>
#include <generic/vfs/Error.hpp>

#include <librt/Optional.hpp>
#include <librt/String.hpp>
#include <librt/StringRef.hpp>
#include <librt/containers/Map.hpp>
#include <librt/SharedPtr.hpp>
#include <librt/UniquePtr.hpp>
#include <librt/Assert.hpp>

namespace core::vfs
{
  class Vnode : public rt::SharedPtrHook
  {
  public:
    Vnode(Vnode* parent) : m_parent(parent) {}
    void associate(rt::SharedPtr<Inode> inode) { ASSERT(m_state == State::NEW); m_state = State::NORMAL; m_inode = rt::move(inode); }

  public:
    const SuperBlock& superBlock() const { return m_inode->superBlock(); }
    SuperBlock& superBlock()             { return m_inode->superBlock(); }

  public:
    const Inode& inode() const { return *m_inode; }
    Inode& inode()             { return *m_inode; }

  public:
    bool negative() const { ASSERT(m_state == State::NORMAL || m_state == State::MOUNTED); return !m_inode; }

  public:
    Result<void> mount(Mountable& mountable, rt::Span<rt::StringRef> args);
    Result<void> umount();

  public:
    rt::SharedPtr<Vnode>& lookup_ref(rt::StringRef name);
    Result<rt::SharedPtr<Vnode>> lookup(rt::StringRef name);

  public:
    Result<rt::SharedPtr<Vnode>> create(rt::StringRef name, Type type);
    Result<void> link(rt::StringRef name, Inode& inode); // FIXME:
    Result<void> unlink(rt::StringRef name);

  private:
    Vnode* m_parent; // To support walking .. directories
    rt::containers::Map<rt::String, rt::SharedPtr<Vnode>> m_childs;

  private:
    enum class State { NEW, NORMAL, UMOUNTED, MOUNTED } m_state = State::NEW;
    rt::SharedPtr<Inode> m_inode;
  };
}

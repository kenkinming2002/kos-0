#pragma once

#include <generic/vfs/Vnode.hpp>

#include <librt/SharedPtr.hpp>

namespace core::vfs
{
  /* TODO: Add permision checking */
  class File
  {
  public:
    File(rt::SharedPtr<Vnode> vnode);

  public:
    using Stat = Inode::Stat;
    Result<Stat> stat();

  /******************
   * File interface *
   ******************/
  public:
    Result<size_t> read(char* buf, size_t length, addr_t off);
    Result<size_t> write(const char* buf, size_t length, addr_t off);
    Result<void>   resize(size_t size);

  public:
    using DirectoryEntry     = Inode::DirectoryEntry;
    using iterate_callback_t = Inode::iterate_callback_t;
    Result<void> iterate(iterate_callback_t cb, void* data);

  /*******************
   * Vnode interface *
   *******************/
  public:
    Result<void> mount(Mountable& mountable, rt::Span<rt::StringRef> args);
    Result<void> umount();

  public:
    Result<File> lookup(rt::StringRef name);
    Result<File> create(rt::StringRef name, Type type);
    Result<void> link(rt::StringRef name, Inode& inode); // FIXME
    Result<void> unlink(rt::StringRef name);

  private:
    rt::SharedPtr<Vnode> m_vnode; // Keep track of path information
  };
}

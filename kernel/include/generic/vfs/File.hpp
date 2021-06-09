#pragma once

#include <generic/vfs/Vnode.hpp>

#include <librt/SharedPtr.hpp>

namespace core::vfs
{
  /* TODO: Add permision checking */
  class File
  {
  public:
    using Stat = Inode::Stat;

  public:
    constexpr File() = default;
    constexpr File(rt::SharedPtr<Vnode> vnode) : m_vnode(rt::move(vnode)) {}

  public:
    bool isOpen() const { return m_vnode; }
    void close();

  public:
    Result<Stat> stat();

  /******************
   * File interface *
   ******************/
  public:
    /* We use ssize_t instead of size_t as this allow us to use negative number
     * to return error code to user space */
    Result<ssize_t> seek(Anchor anchor, off_t offset);

  public:
    Result<ssize_t> read(char* buf, size_t length);
    Result<ssize_t> write(const char* buf, size_t length);
    Result<void>   resize(size_t size);

  public:
    Result<ssize_t> readdir(char* buf, size_t length);

  /*******************
   * Vnode interface *
   *******************/
  public:
    Result<void> mount(Mountable& mountable, rt::StringRef arg);
    Result<void> umount();

  public:
    Result<File> lookup(rt::StringRef name);
    Result<File> create(rt::StringRef name, Type type);
    Result<void> link(rt::StringRef name, Inode& inode); // FIXME
    Result<void> unlink(rt::StringRef name);

  private:
    rt::SharedPtr<Vnode> m_vnode; // Keep track of path information
    size_t m_pos = 0;
  };
}

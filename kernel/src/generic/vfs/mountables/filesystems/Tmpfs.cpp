#include <generic/vfs/mountables/filesystems/Tmpfs.hpp>

#include <librt/SharedPtr.hpp>

namespace core::vfs
{
  rt::Result<rt::SharedPtr<Inode>, ErrorCode> Tmpfs::mount(rt::Span<rt::StringRef> args)
  {
    auto superBlock = rt::makeShared<TmpfsSuperBlock>();
    return rt::SharedPtr<Inode>(superBlock->allocate(superBlock, Type::DIRECTORY));
  }

  rt::StringRef Tmpfs::name()
  {
    return "tmpfs";
  }

  Result<SuperBlock::Stat> TmpfsSuperBlock::stat()
  {
    return SuperBlock::Stat{};
  }

  rt::SharedPtr<TmpfsInode> TmpfsSuperBlock::allocate(rt::SharedPtr<TmpfsSuperBlock> self, Type type)
  {
    switch(type)
    {
    case Type::DIRECTORY:
      return rt::makeShared<TmpfsDirectoryInode>(rt::move(self), m_next++);
    case Type::REGULAR_FILE:
      return rt::makeShared<TmpfsFileInode>(rt::move(self), m_next++);
    default:
      ASSERT_UNREACHABLE;
    }
  }

  TmpfsInode::TmpfsInode(rt::SharedPtr<TmpfsSuperBlock> superBlock, ino_t ino)
    : m_superBlock(rt::move(superBlock)), m_ino(ino) {}

  const TmpfsSuperBlock& TmpfsInode::superBlock() const { return *m_superBlock; }
  TmpfsSuperBlock& TmpfsInode::superBlock()             { return *m_superBlock; }

  rt::SharedPtr<TmpfsInode> TmpfsInode::allocate(Type type)
  {
    return m_superBlock->allocate(m_superBlock, type);
  }

  Result<Inode::Stat> TmpfsInode::genericStat(Type type, size_t size)
  {
    return Stat{
      .dev        = 0,
      .ino        = m_ino,
      .type       = type,
      .blockSize  = 1,
      .size       = size,
      .moduleName = "tmpfs",
      .deviceName = "tmpfs"
    };
  }

  Result<Inode::Stat> TmpfsDirectoryInode::stat() { return genericStat(Type::DIRECTORY, 0); }

  Result<void> TmpfsDirectoryInode::iterate(iterate_callback_t cb, void* data)
  {
    for(auto& [name, inode] : m_childs)
    {
      auto stat = inode->stat();
      if(!stat)
        return stat.error();
      cb(DirectoryEntry{.name = name, .ino = stat->ino, .type = stat->type}, data);
    }

    return {};
  }

  Result<rt::SharedPtr<Inode>> TmpfsDirectoryInode::lookup(rt::StringRef name)
  {
    auto it = m_childs.find(name);
    if(it == m_childs.end())
      return ErrorCode::NOT_EXIST;

    return rt::SharedPtr<Inode>(it->second);
  }

  Result<rt::SharedPtr<Inode>> TmpfsDirectoryInode::create(rt::StringRef name, Type type)
  {
    auto inode = allocate(type);
    m_childs.insert({name, inode});
    return rt::SharedPtr<Inode>(move(inode));
  }

  Result<void> TmpfsDirectoryInode::link(rt::StringRef name, rt::SharedPtr<Inode> inode)
  {
    return ErrorCode::UNSUPPORTED;
  }

  Result<void> TmpfsDirectoryInode::unlink(rt::StringRef name)
  {
    auto it = m_childs.find(name);
    ASSERT(it != m_childs.end());
    m_childs.erase(it);
    return {};
  }

  Result<Inode::Stat> TmpfsFileInode::stat() { return genericStat(Type::REGULAR_FILE, m_size); }

  Result<size_t> TmpfsFileInode::read(char* buf, size_t length, addr_t off)
  {
    if(m_size<off)
      return ErrorCode::INVALID;

    length = rt::min(m_size-off, length);
    rt::copy_n(m_data.get()+off, buf, length);
    return length;
  }

  Result<size_t> TmpfsFileInode::write(const char* buf, size_t length, addr_t off)
  {
    if(m_size<off)
      return ErrorCode::INVALID;

    length = rt::min(m_size-off, length);

    rt::copy_n(buf, m_data.get()+off, length);
    return length;
  }

  Result<void> TmpfsFileInode::resize(size_t size)
  {
    auto newData = rt::makeUnique<char[]>(size, '\0');
    if(!newData)
      return ErrorCode::OUT_OF_MEMORY;

    if(m_data)
      rt::copy_n(m_data.get(), newData.get(), rt::min(m_size, size));

    m_data = rt::move(newData);
    m_size = size;

    return {};
  }
}

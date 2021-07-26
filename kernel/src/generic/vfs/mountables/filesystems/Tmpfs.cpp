#include <generic/vfs/mountables/filesystems/Tmpfs.hpp>

#include <i686/syscalls/Access.hpp>

#include <librt/Global.hpp>

namespace core::vfs
{
  namespace
  {
    constinit rt::Global<Tmpfs> tmpfs; // Tmpfs is always available because that is the default root mountable
  }

  void initializeTmpfs()
  {
    tmpfs.construct();
    registerMountable(tmpfs());
  }

  Result<rt::SharedPtr<Inode>> Tmpfs::mount(rt::StringRef arg)
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
    // FIXME: detect m_next wrap around
    switch(type)
    {
    case Type::DIRECTORY:
      return rt::makeShared<TmpfsDirectoryInode>(rt::move(self), m_next++);
    case Type::REGULAR_FILE:
      return rt::makeShared<TmpfsFileInode>(rt::move(self), m_next++);
    case Type::SYMBOLIC_LINK:
    case Type::OTHER:
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

  Result<ssize_t> TmpfsDirectoryInode::readdir(char* buf, size_t length)
  {
    auto buffer = syscalls::OutputUserBuffer(buf, length);
    for(auto& [name, inode] : m_childs)
    {
      // TODO: align the buffer
      auto stat = inode->stat();
      if(!stat)
        return stat.error();

      Dirent dirent = {};
      dirent.length = sizeof(Dirent) + name.length() + 1;
      dirent.ino    = static_cast<uword_t>(stat->ino);
      dirent.type   = static_cast<uword_t>(stat->type);

      if(auto result = buffer.writeAll(dirent, name); !result)
        return result.error();
    }

    return length - buffer.length();
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
    if(type != Type::REGULAR_FILE && type != Type::DIRECTORY)
      return ErrorCode::INVALID;

    auto inode = allocate(type);
    m_childs.insert({rt::String(name), inode});
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

  Result<ssize_t> TmpfsFileInode::read(char* buf, size_t length, size_t pos)
  {
    if(m_size<pos)
      return ErrorCode::INVALID;

    length = rt::min(m_size-pos, length);
    rt::copy_n(m_data.get()+pos, buf, length);

    return length;
  }

  Result<ssize_t> TmpfsFileInode::write(const char* buf, size_t length, size_t pos)
  {
    if(m_size<pos)
      return ErrorCode::INVALID;

    length = rt::min(m_size-pos, length);
    rt::copy_n(buf, m_data.get()+pos, length);

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

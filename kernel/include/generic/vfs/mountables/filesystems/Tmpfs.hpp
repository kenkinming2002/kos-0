#include <generic/vfs/Mountable.hpp>
#include <generic/vfs/Inode.hpp>
#include <generic/vfs/Error.hpp>

#include <librt/StringRef.hpp>
#include <librt/SharedPtr.hpp>
#include <librt/containers/Map.hpp>

namespace core::vfs
{
  class Tmpfs;
  class TmpfsSuperBlock;
  class TmpfsInode;

  class Tmpfs final : public Mountable
  {
  public:
    rt::Result<rt::SharedPtr<Inode>, ErrorCode> mount(rt::Span<rt::StringRef> args) override;
    rt::StringRef name() override;
  };

  class TmpfsSuperBlock final : public SuperBlock
  {
  public:
    Result<Stat> stat() override;

  public:
    rt::SharedPtr<TmpfsInode> allocate(rt::SharedPtr<TmpfsSuperBlock> self, Type type);

  private:
    ino_t m_next = 0;
  };

  class TmpfsInode : public Inode
  {
  public:
    TmpfsInode(rt::SharedPtr<TmpfsSuperBlock> superBlock, ino_t ino);

  public:
    const TmpfsSuperBlock& superBlock() const override final;
    TmpfsSuperBlock& superBlock() override final;

  protected:
    rt::SharedPtr<TmpfsInode> allocate(Type type);
    Result<Stat> genericStat(Type type, size_t size);

  private:
    rt::SharedPtr<TmpfsSuperBlock> m_superBlock;
    ino_t m_ino;
  };


  class TmpfsDirectoryInode final : public TmpfsInode
  {
  public:
    using TmpfsInode::TmpfsInode;

  public:
    Result<Stat> stat() override;

  public:
    Result<void> iterate(iterate_callback_t cb, void* data) override;

  public:
    Result<rt::SharedPtr<Inode>> lookup(rt::StringRef name) override;
    Result<rt::SharedPtr<Inode>> create(rt::StringRef name, Type type) override;
    Result<void> link(rt::StringRef name, rt::SharedPtr<Inode> inode) override;
    Result<void> unlink(rt::StringRef name) override;

  private:
    rt::containers::Map<rt::String, rt::SharedPtr<TmpfsInode>> m_childs;
  };

  class TmpfsFileInode final : public TmpfsInode
  {
  public:
    using TmpfsInode::TmpfsInode;

  public:
    Result<Stat> stat() override;

  public:
    Result<size_t> read(char* buf, size_t length, addr_t off) override;
    Result<size_t> write(const char* buf, size_t length, addr_t off) override;
    Result<void> resize(size_t size) override;

  private:
    /* Replace with rt::Vector, which we should implement soon */
    size_t m_size;
    rt::UniquePtr<char[]> m_data;
  };
}

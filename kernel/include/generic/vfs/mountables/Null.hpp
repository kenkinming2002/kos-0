#pragma once

#include <generic/vfs/Inode.hpp>

namespace core::vfs
{
  class NullSuperBlock : public SuperBlock { };
  class NullInode : public Inode
  {
  public:
    virtual const SuperBlock& superBlock() const { return m_nullSuperBlock; }
    virtual SuperBlock& superBlock()             { return m_nullSuperBlock; }

  private:
    NullSuperBlock m_nullSuperBlock;
  };
}

#include <generic/memory/MemoryArea.hpp>

namespace core::memory
{
  namespace { using PageFrame = char[PAGE_SIZE]; }

  Result<physaddr_t> MemoryArea::getPageFrame(uintptr_t addr)
  {
    /* FIXME: Also copy dirty m_pages since they are already marked writable, and
     *        making it read-only again could have a lot of race condition */
    if(auto it = m_pages.find(addr); it != m_pages.end())
      return it->second->physaddr();

    /* Page allocation */
    auto _page = Page::allocate();
    if(!_page)
      return ErrorCode::OUT_OF_MEMORY;

    auto it = m_pages.insert({addr, rt::move(_page)});
    ASSERT(it != m_pages.end());
    auto& page = it->second;

    auto& pageFrame = *static_cast<PageFrame*>(page->ptr);
    rt::fill(rt::begin(pageFrame), rt::end(pageFrame), '\0');
    if(file)
    {
      auto addrOffset = addr - this->addr;
      auto fileOffset = this->fileOffset+addrOffset;
      auto fileLength = addrOffset           > this->fileLength ? 0
                      : addrOffset+PAGE_SIZE < this->fileLength ? PAGE_SIZE
                      : this->fileLength-addrOffset;

      file->seek(Anchor::BEGIN, fileOffset);
      file->read(pageFrame, fileLength); /* Discard error */
    }

    return page->physaddr();
  }

  Result<physaddr_t> MemoryArea::getWritablePageFrame(uintptr_t addr)
  {
    // This is where we could implement COW semantics
    auto it = m_pages.find(addr);
    ASSERT(it != m_pages.end());
    auto& page = it->second;

    if(type == MapType::PRIVATE && page.count() != 1)
    {
      rt::log("Copy on write triggered\n");
      // Copy on write
      auto _newPage = Page::allocate();
      if(!_newPage)
        return ErrorCode::OUT_OF_MEMORY;

      auto oldPage = rt::exchange(page, _newPage);
      auto& oldPageFrame = *static_cast<PageFrame*>(oldPage->ptr);
      auto& pageFrame    = *static_cast<PageFrame*>(page->ptr);
      rt::copy(rt::begin(oldPageFrame), rt::end(oldPageFrame), rt::begin(pageFrame));
    }


    page->dirty = true;
    return page->physaddr();
  }

  void MemoryArea::removePageFrame(uintptr_t addr)
  {
    auto it = m_pages.find(addr);
    ASSERT(it != m_pages.end());
    m_pages.erase(it);
  }

}

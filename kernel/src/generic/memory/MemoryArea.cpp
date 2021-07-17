#include <generic/memory/MemoryArea.hpp>

namespace core::memory
{
  namespace { using PageFrame = char[PAGE_SIZE]; }

  Result<physaddr_t> MemoryArea::getPageFrame(uintptr_t addr)
  {
    if(auto it = pages.find(addr); it != pages.end())
      return it->second->physaddr();

    auto _page = Page::allocate();
    if(!_page)
      return ErrorCode::OUT_OF_MEMORY;

    auto it = pages.insert({addr, rt::move(_page)});
    ASSERT(it != pages.end());
    auto& page = it->second;

    auto& pageFrame = *static_cast<PageFrame*>(page->ptr);
    rt::fill(rt::begin(pageFrame), rt::end(pageFrame), '\0');
    if(file)
    {
      auto offset = this->offset+(addr-this->addr);
      file->seek(Anchor::BEGIN, offset);
      file->read(pageFrame, sizeof pageFrame); // We do not care, if it failed, we are left with an empty page
    }

    return page->physaddr();
  }

  Result<physaddr_t> MemoryArea::getWritablePageFrame(uintptr_t addr)
  {
    // This is where we could implement COW semantics
    auto it = pages.find(addr);
    ASSERT(it != pages.end());
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
    auto it = pages.find(addr);
    ASSERT(it != pages.end());
    pages.erase(it);
  }

}

#include <algorithm>
#include <core/generic/memory/LinkedListPagesAllocator.hpp>


namespace core::memory
{
  std::optional<Pages> LinkedListPagesAllocator::allocate(size_t count)
  {
    for(auto& pages : m_pagesList)
      if(pages.count>=count)
      {
        auto result = Pages{pages.index, count};

        pages.index+=count;
        pages.count-=count;

        return result;
      }

    return std::nullopt;
  }

  void LinkedListPagesAllocator::deallocate(Pages target)
  {
    markAsAvailable(target);
  }

  void LinkedListPagesAllocator::markAsAvailable(Pages target)
  {
    for(auto it=m_pagesList.begin(); it!=m_pagesList.end(); ++it)
    {
      const auto& pages = *it;
      if(target.index+target.count<=pages.index)
      {
        m_pagesList.insert(it, target);
        return;
      }
    }
    m_pagesList.insert(m_pagesList.end(), target);

    // FIXME: The following code does not work and cause a triple fault.
    //        It's probably accessing memory it shouldn't be accessing.
    //        The issue could be in the erase function, but we will see.
    //
    // TODO: Defragmentation
    //for(auto it=m_pagesList.begin(); it!=std::prev(m_pagesList.end()); ++it)
    //{
    //  auto nextIt = std::next(it);
    //  auto &pages = *it, &nextPages = *nextIt;
    //  if(pages.index+pages.count==nextPages.index)
    //  {
    //    pages.count+=nextPages.count;
    //    m_pagesList.erase(nextIt);
    //  }
    //}
  }

  void LinkedListPagesAllocator::markAsUsed(Pages target)
  {
    auto it = std::find_if(m_pagesList.begin(), m_pagesList.end(), [&](auto& pages){
      return pages.index<=target.index && target.index+target.count<=pages.index+pages.count;
    });
    auto& pages = *it;
    auto frontPages = Pages{pages.index, target.index-pages.index};
    auto backPages  = Pages{target.index+target.count, (pages.index+pages.count)-(target.index+target.count)};
    if(frontPages.count!=0 && backPages.count !=0)
    {
      pages = backPages;
      m_pagesList.insert(it, frontPages);
    }
    else if(frontPages.count!=0)
      pages = frontPages;
    else if(backPages.count!=0)
      pages = backPages;
    else
      m_pagesList.erase(it);
  }
}

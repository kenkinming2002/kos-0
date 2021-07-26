#pragma once

#include <librt/StringRef.hpp>
#include <librt/Pair.hpp>

namespace core::vfs::path
{
  /*
   * This is needed for compatiblity with some program
   *
   * According to POSIX standard, a path with trailing slash such as "abc/def/"
   * should be interpreted as "abc/def/." with a dot added at the end. In such a
   * case, the dirname is "abc/def" and the basename is ".". When a path is
   * need to be look up, the dirname should be followed and thus must exist and
   * is a directory, whereas the file/directory with basename may exist
   * depending on the exact syscall.  Thus, in such case, syscall such as
   * mkdir('abc/def/') should never succeed and should be replace with
   * mkdir('abc/def') instead. However to maintain compatiblity with program
   * that depends on such behavior that the trailing slash is ignored, as in the
   * case of linux, we would need to trim the trailing slash in syscall that
   * have to do with directories.
   */
  inline constexpr auto trimTrailingSlash(rt::StringRef path)
  {
    while(!path.empty() && path[path.length()-1] == '/')
      path = path.removeSuffix(1);

    return path;
  }

  inline constexpr auto trimLeadingSlash(rt::StringRef path)
  {
    while(!path.empty() && path[0] == '/')
      path = path.removePrefix(1);

    return path;
  }

  inline constexpr auto splitFirst(rt::StringRef path)
  {
    path = trimLeadingSlash(path);
    auto it = rt::find(path.begin(), path.end(), '/');
    if(it != path.end())
      return rt::Pair(rt::StringRef(path.begin(), it), rt::StringRef(it+1, path.end()));
    else
      return rt::Pair(path, rt::StringRef());
  }

  inline auto splitLast(rt::StringRef path)
  {
    path = trimTrailingSlash(path);
    auto it = rt::find(path.rbegin(), path.rend(), '/');
    if(it != path.rend())
      return rt::Pair(rt::StringRef(path.begin(), &(*it)), rt::StringRef(&(*it)+1, path.end()));
    else
      return rt::Pair(rt::StringRef(), path);
  }

  inline constexpr auto popFirst(rt::StringRef& path)
  {
    auto [first, last] = splitFirst(path);
    path = last;
    return first;
  }

  inline constexpr auto popLast(rt::StringRef& path)
  {
    auto [first, second] = splitFirst(path);
    path = second;
    return first;
  }

  inline constexpr bool isAbsolute(rt::StringRef path)
  {
    return !path.empty() && path[0] == '/';
  }

  namespace details
  {
    struct PathView
    {
    public:
      PathView(rt::StringRef path) : m_path(path) {}

    public:
      struct Sentinel {};
      struct Iterator
      {
      public:
        constexpr Iterator() = default;
        constexpr Iterator(rt::StringRef path) : m_path(isAbsolute(path) ? path.removePrefix(1) : path), m_result()
        {
          ++(*this);
        }

      public:
        auto operator*() { return m_result; }

      public:
        inline Iterator& operator++()
        {
          if(!m_path.empty())
          {
            auto it = rt::find(m_path.begin(), m_path.end(), '/');
            if(it != m_path.end())
            {
              m_result = rt::StringRef(m_path.begin(), it);
              m_path   = rt::StringRef(rt::next(it), m_path.end());
            }
            else
            {
              m_result = rt::StringRef(m_path.begin(), it);
              m_path   = rt::StringRef();
            }
          }
          else
            m_result = rt::StringRef(); // Set result to empty to signifiy the end

          return *this;
        }

        Iterator operator++(int)
        {
          auto tmp = *this;
          ++(*this);
          return tmp;
        }

        bool operator==(Sentinel) const
        {
          // It could happen that we have a `weird' path such as a//c
          // and so that a empty result does not always signify the end
          // and we have to check whether m_path is empty
          return m_path.empty() && m_result.empty();
        }

      private:
        rt::StringRef m_path;
        rt::StringRef m_result;
      };

    public:
      auto begin() const { return Iterator(m_path); }
      auto end()   const { return Sentinel(); }

    private:
      rt::StringRef m_path;
    };
  }

  inline auto components(rt::StringRef path) { return details::PathView(path); }
}

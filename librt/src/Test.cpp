#include <librt/String.hpp>
#include <librt/StringRef.hpp>

#include <librt/UniquePtr.hpp>
#include <librt/SharedPtr.hpp>

#include <librt/Optional.hpp>
#include <librt/Variant.hpp>
#include <librt/Result.hpp>

#include <librt/containers/Map.hpp>
#include <librt/containers/List.hpp>

#include <librt/Assert.hpp>

#include <thread>
#include <vector>

namespace rt::hooks
{
  void* malloc(size_t size) { return ::malloc(size); }
  void free(void* ptr)      { return ::free(ptr); }
  [[noreturn]] void abort() { ::abort(); }
  void log(const char* str, size_t length) { printf("%.*s", (int)length, str); fflush(stdout); }
}

static void testString()
{
  ASSERT_TRUE(rt::String("test3") < rt::String("test4"));
  ASSERT_FALSE(rt::String("test3") > rt::String("test4"));
  ASSERT(rt::String("Hello") < rt::String("HelloS"));
}

static void testOptional()
{
  rt::Optional<int> a(1), b(rt::nullOptional);
  ASSERT_TRUE(a);
  ASSERT_FALSE(b);
}

static void testResult()
{
  enum class ErrorCode { ERROR1 = 123, ERROR2 = 999 };
  rt::Result<int, ErrorCode> a(1), b(ErrorCode::ERROR2);
  ASSERT_TRUE(a);
  ASSERT_FALSE(b);

  ASSERT(*a == 1);
  ASSERT(b.error() == ErrorCode::ERROR2);
}

static void testVariant()
{
  rt::Variant<int, rt::String, char> v;
  ASSERT(v.empty());

  v.emplace(rt::type_constant<int>, 77);
  ASSERT(!v.empty());
  ASSERT(v.holds<int>());
  ASSERT(v.get<int>() == 77);

  v.emplace(rt::type_constant<rt::String>, "HelloWorld");
  ASSERT(!v.empty());
  ASSERT(v.holds<rt::String>());
  ASSERT(v.get<rt::String>() == rt::StringRef("HelloWorld"));

  v.emplace(rt::type_constant<char>, 34);
  ASSERT(!v.empty());
  ASSERT(v.holds<char>());
  ASSERT(v.get<char>() == 34);

  v.reset();
  ASSERT(v.empty());
}

static void testMap()
{
  rt::containers::Map<int, int> map;

  map.insert(rt::Pair(3, 3));
  map.insert(rt::Pair(1, 1));
  map.insert(rt::Pair(2, 3));
  map.insert(rt::Pair(6, 3));
  map.insert(rt::Pair(4, 3));
  ASSERT(map.find(3) != map.end());
  ASSERT(map.find(1) != map.end());
  ASSERT(map.find(2) != map.end());
  ASSERT(map.find(6) != map.end());
  ASSERT(map.find(4) != map.end());
  ASSERT(map.find(7) == map.end());

  map.erase(++(++map.begin()));
  map.erase(++(++map.begin()));
  ASSERT(map.find(3) == map.end());
  ASSERT(map.find(1) != map.end());
  ASSERT(map.find(2) != map.end());
  ASSERT(map.find(6) != map.end());
  ASSERT(map.find(4) == map.end());
}

static void testList()
{
  rt::containers::List<int> list;
  ASSERT(list.empty());

  list.insert(list.begin(), 1);
  list.insert(list.begin(), 2);
  list.insert(list.begin(), 3);
  list.insert(list.begin(), 4);
  ASSERT(*list.begin() == 4);
  ASSERT(*(++list.begin()) == 3);
  ASSERT(*(++(++list.begin())) == 2);

  list.erase(++list.begin());
  ASSERT(*list.begin() == 4);
  ASSERT(*(++list.begin()) == 2);
  ASSERT(*(++(++list.begin())) == 1);
}

static void testSharedPtr()
{
  struct Test : public rt::SharedPtrHook
  {
    int value;
  };

  const rt::SharedPtr<Test> ptr(new Test{.value = 1});
  ASSERT(ptr->value == 1);
  ASSERT(ptr.count() == 1);
  {
    auto copy = ptr;
    ASSERT(ptr->value == 1);
    ASSERT(copy->value == 1);
    ASSERT(&ptr->value == &copy->value);
    ASSERT(ptr.count() == 2);
    ASSERT(copy.count() == 2);
  }

  std::vector<std::thread> threads;
  for(size_t i=0; i<std::thread::hardware_concurrency(); ++i)
    threads.emplace_back([=](){ for(size_t i=0; i<10; ++i) { auto dup = ptr;} });

  for(auto& thread : threads)
    thread.join();
}

int main()
{
  testString();
  testOptional();
  testResult();
  testVariant();
  testMap();
  testList();
  testSharedPtr();
  return 0;
}

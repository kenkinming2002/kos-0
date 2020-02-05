#ifndef MACRO_CAST_H
#define MACRO_CAST_H

//#include <type_traits>
//
//namespace utils
//{
//  template<typename T, typename = std::void_t<std::enable_if_t<std::is_enum_v<T>>>>
//  auto unsigned_enum_cast(T t)
//  {
//    using underlying_type = std::underlying_type_t<T>;
//    using type = std::make_unsigned_t<underlying_type>;
//    return static_cast<type>(t);
//  }
//}

#define FORCE_CAST(type, var) *(type*)&var

#endif // MACRO_CAST_H

#pragma once

#if defined(ARCHITECTURE_i686)
  #if __has_include(<i686/FILE_NAME>)
    #include <i686/FILE_NAME>
  #elif __has_include(<intel/FILE_NAME>)
    #include <intel/FILE_NAME>
  #elif __has_include(<generic/FILE_NAME>)
    #include <generic/FILE_NAME>
  #else
    #error Header Not Found
  #endif
#elif defined(ARCHITECTURE_x86_64)
  #if __has_include(<i686/FILE_NAME>)
    #include <x86_64/FILE_NAME>
  #elif __has_include(<intel/FILE_NAME>)
    #include <intel/FILE_NAME>
  #elif __has_include(<generic/FILE_NAME>)
    #include <generic/FILE_NAME>
  #else
    #error Header Not Found
  #endif
#endif

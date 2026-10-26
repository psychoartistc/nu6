#pragma once

#include <cstdarg>
#include <string_view>

namespace utils {
[[noreturn]] void vpanic(std::string_view fmt, std::va_list list);
[[noreturn]] void panic(std::string_view fmt, ...);
} // namespace utils

#define assert(condition)                                                      \
  do {                                                                         \
    if (!(condition)) {                                                        \
      utils::panic("assertion failed: %s, file %s, line %d", #condition,       \
                   __FILE__, __LINE__);                                        \
    }                                                                          \
  } while (0)

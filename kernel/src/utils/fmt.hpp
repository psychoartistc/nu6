#pragma once

#include <vendor/printf.hpp>
#include <cstddef>
#include <array>

namespace utils {

static inline constexpr int fmt_sizec(char *to, size_t bytes) {
  static const char units[] = "BKMGTPE";
  size_t value = bytes;
  size_t unit = 0;
  int outputted = 0;

  while (value >= 1024 && unit < sizeof(units) - 2) {
    value /= 1024;
    unit++;
  }

  if (unit == 0) {
    outputted += sprintf_(to, "%zuB", bytes);
    return outputted;
  }

  size_t divisor = 1;
  for (size_t i = 0; i < unit; i++)
    divisor *= 1024;

  size_t whole = bytes / divisor;
  size_t rem = bytes % divisor;
  size_t tenth = (rem * 10) / divisor;
  if (tenth == 0) {
    outputted += sprintf_(to, "%zu%c", whole, units[unit]);
  } else
    outputted += sprintf_(to, "%zu.%zu%c", whole, tenth, units[unit]);

  to[outputted] = '\0';
  return outputted;
}

inline constexpr std::size_t FMT_SIZE_BUFSIZE =
    ((sizeof(size_t) * 8 * 30103 + 99999) / 100000 + 2);

template <size_t bufsize>
int fmt_size(std::array<char, bufsize> &buff, std::size_t bytes)
  requires(bufsize >= FMT_SIZE_BUFSIZE)

{
  return fmt_sizec(buff.data(), bytes);
}

} // namespace utils

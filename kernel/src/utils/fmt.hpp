#pragma once
#include <cstddef>
#include <array>

namespace utils {

template <size_t bufsize>
void fmt_size(std::array<char, bufsize> buff)
  requires(bufsize >= ((sizeof(size_t) * 8 * 30103 + 99999) / 100000 + 2))

{}

} // namespace utils

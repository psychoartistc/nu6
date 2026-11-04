#pragma once

#include <span>
#include <cstddef>
#include <cstdint>

namespace utils {
class rng {
private:
public:
  rng();

  void fill_bytes(std::span<std::uint8_t> span);
};
} // namespace utils

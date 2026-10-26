#pragma once

#include <mm/common.hpp>
#include <cstdint>
#include <concepts>

namespace utils {

template <std::integral T> constexpr T div_floor(T a, T b) {
  T q = a / b;
  T r = a % b;
  bool same_sign = (a >= 0) == (b >= 0);
  if (r != 0 && !same_sign)
    --q;

  return q;
}

template <std::integral T> static constexpr T align_up(T v, T a) noexcept {
  return (v + a - 1) & ~(a - 1);
}

static inline constexpr bool page_aligned(std::uintptr_t ptr) {
  return (ptr & mm::PAGE_SIZE) == 0;
}

static inline constexpr bool superpage_aligned(std::uintptr_t ptr) {
  return (ptr & (mm::SUPERPAGE_SIZE - 1)) == 0;
}

static inline constexpr bool megapage_aligned(std::uintptr_t ptr) {
  return (ptr & (mm::MEGAPAGE_SIZE - 1)) == 0;
}
} // namespace utils

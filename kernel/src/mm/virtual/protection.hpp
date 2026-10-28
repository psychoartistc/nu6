#pragma once

#include <type_traits>
#include <cstdint>

namespace mm {

enum class protection {
  none = 0,
  read = 1 << 0,
  write = 1 << 1,
  execute = 1 << 2,
  nocache = 1 << 3,
  kernel = 1 << 4
};

inline protection operator|(protection lhs, protection rhs) {
  using T = std::underlying_type_t<protection>;
  return static_cast<protection>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline protection operator&(protection lhs, protection rhs) {
  using T = std::underlying_type_t<protection>;
  return static_cast<protection>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

inline protection operator~(protection val) {
  using T = std::underlying_type_t<protection>;
  return static_cast<protection>(~static_cast<T>(val));
}

inline protection &operator|=(protection &lhs, protection rhs) {
  lhs = lhs | rhs;
  return lhs;
}

static inline constexpr bool protection_has_flag(protection prot,
                                                 protection targ) {
  return (prot & targ) != protection::none;
}

static inline constexpr bool pte_has_flag(std::uint64_t pte,
                                          std::uint64_t flag) {
  return pte & flag;
}

static inline constexpr bool prot_violates_wxorx(protection prot) {
  return protection_has_flag(prot, protection::write) &&
         protection_has_flag(prot, protection::execute);
}
} // namespace mm

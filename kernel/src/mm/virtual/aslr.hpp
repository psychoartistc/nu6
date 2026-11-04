#pragma once

#include <utils/rng.hpp>

namespace mm {
class aslr_manager {
public:
  static inline constexpr std::uintptr_t USER_VMA_START = 0x0000000000000000;
  static inline constexpr std::uintptr_t USER_VMA_END = 0x00007fffffffffff;

  static inline constexpr std::uintptr_t KERNEL_VMA_START = 0xffffc90000000000;
  static inline constexpr std::uintptr_t KERNEL_VMA_END = 0xffffe90000000000;

  static inline constexpr std::size_t USER_VMA_SIZE =
      (64ULL * 1024 * 1024 * 1024 * 1024);

  static inline constexpr std::uintptr_t KERNEL_VMA_SIZE =
      (16ULL * 1024 * 1024 * 1024 * 1024);

private:
  static utils::rng s_rng;

private:
  static std::uint64_t random_u64();
  static std::uintptr_t random_offset(std::uintptr_t max);

public:
  static void initialize();

  static std::pair<std::uintptr_t, std::uintptr_t> generate_kernel();
  static std::pair<std::uintptr_t, std::uintptr_t> generate_user();
};
} // namespace mm

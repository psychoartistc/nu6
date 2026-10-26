#pragma once

#include <cstddef>
#include <cstdint>

namespace mm {
static inline constexpr std::uintptr_t USER_VMA_START = 0x0000000000000000;
static inline constexpr std::uintptr_t USER_VMA_END = 0x00007fffffffffff;

static inline constexpr std::uintptr_t KERNEL_VMA_START = 0xffffc90000000000;
static inline constexpr std::uintptr_t KERNEL_VMA_END = 0xffffe90000000000;

static inline constexpr std::size_t USER_VMA_SIZE =
    (128ULL * 1024 * 1024 * 1024 * 1024);

static inline constexpr std::uintptr_t KERNEL_VMA_SIZE =
    (32ULL * 1024 * 1024 * 1024 * 1024);

} // namespace mm

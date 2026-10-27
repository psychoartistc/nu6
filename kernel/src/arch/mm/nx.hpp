#pragma once

#include <arch/io/msr.hpp>

namespace arch {
namespace mm {
static inline constexpr std::uint32_t IA32_EFER = 0xC0000080;
static inline constexpr std::uint64_t IA32_EFER_NXE = (1 << 11);
bool nx_bit_supported();
void nx_bit_enable();
} // namespace mm
} // namespace arch

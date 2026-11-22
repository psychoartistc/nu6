#pragma once

#include <arch/cpuid.hpp>
#include <cstddef>
#include <cstdint>

namespace arch {
namespace tsc {
std::uint64_t read();
bool invariant_supported();
std::uint64_t herz_from_cpuid();
} // namespace tsc
} // namespace arch

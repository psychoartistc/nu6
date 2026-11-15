#pragma once

#include <arch/cpuid.hpp>
#include <cstddef>
#include <cstdint>

namespace arch {
namespace tsc {
std::uint64_t read();
bool invariant_supported();
} // namespace tsc
} // namespace arch

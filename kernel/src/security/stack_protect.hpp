#pragma once

#include <cstdint>
#include <cstddef>

extern std::uintptr_t __stack_chk_guard;

namespace security {
static inline constexpr std::size_t STACK_CHK_GUARD = 0x595e9fbd94fda766;
}

extern "C" {
[[noreturn]] void __stack_chk_fail(void);
}

#pragma once

#include <cstdint>
#include <arch/control.hpp>

namespace cxxabi {
static inline constexpr std::uint64_t INITIALIZED_MASK = 1ULL << 0;
static inline constexpr std::uint64_t LOCKED_MASK = 1ULL << 1;

extern void (*__init_array[])();
extern void (*__init_array_end[])();

} // namespace cxxabi

extern "C" {
int __cxa_atexit(void (*)(void *), void *, void *);
void __cxa_pure_virtual();
void __cxa_deleted_virtual();

extern void *__dso_handle;

int __cxa_guard_acquire(std::uint64_t *guard);
void __cxa_guard_release(std::uint64_t *guard);
void __cxa_guard_abort(std::uint64_t *guard);
}

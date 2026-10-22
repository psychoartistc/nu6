#include <abi/itanium.hpp>
#include <atomic>

namespace cxxabi {

void call_global_ctors() {
  for (std::size_t i = 0; &__init_array[i] != __init_array_end; i++) {
    __init_array[i]();
  }
}
} // namespace cxxabi

extern "C" {
int __cxa_atexit(void (*)(void *), void *, void *) { return 0; }
void __cxa_pure_virtual() { arch::hcf(); }
void __cxa_deleted_virtual() { arch::hcf(); }

void *__dso_handle;

int __cxa_guard_acquire(std::uint64_t *guard) {
  std::atomic_ref<std::uint64_t> atomic_guard(*guard);

  while (true) {
    std::uint64_t current = atomic_guard.load(std::memory_order_acquire);

    if (current & cxxabi::INITIALIZED_MASK) {
      return 0;
    }

    if (!(current & cxxabi::LOCKED_MASK)) {
      std::uint64_t expected = current;
      std::uint64_t desired = current | cxxabi::LOCKED_MASK;

      if (atomic_guard.compare_exchange_weak(expected, desired,
                                             std::memory_order_acquire,
                                             std::memory_order_relaxed)) {
        return 1;
      }
    }

    arch::pause();
  }
}
void __cxa_guard_release(std::uint64_t *guard) {
  std::atomic_ref<std::uint64_t> atomic_guard(*guard);

  std::uint64_t current = atomic_guard.load(std::memory_order_relaxed);
  std::uint64_t desired =
      (current & ~cxxabi::LOCKED_MASK) | cxxabi::INITIALIZED_MASK;

  atomic_guard.store(desired, std::memory_order_release);
}

void __cxa_guard_abort(std::uint64_t *guard) {
  std::atomic_ref<std::uint64_t> atomic_guard(*guard);

  std::uint64_t current = atomic_guard.load(std::memory_order_relaxed);
  std::uint64_t desired = current & ~cxxabi::LOCKED_MASK;

  atomic_guard.store(desired, std::memory_order_release);
}
}

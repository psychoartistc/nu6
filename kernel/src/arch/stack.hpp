#pragma once

#include <functional>
#include <cstdint>
#include <expected>
#include <cstddef>

namespace arch {
class stack {
public:
  enum class error_code { no_memory };

  static inline constexpr std::size_t KERNEL_STACK_PAGES = 3;
  static inline constexpr std::size_t USER_STACK_PAGES = 256;

  std::uintptr_t m_top{0x0};

public:
  stack(std::uintptr_t top);

  void enter(std::function<void()> fn);

  std::uintptr_t top() const { return m_top; }

  // allocate a new kernel stack
  // will panic if the allocation fails !
  static stack new_kernel();

  // allocate a new user task
  static std::expected<stack, error_code> new_user();
};
} // namespace arch

#include <arch/stack.hpp>
#include <mm/common.hpp>
#include <mm/stl.hpp>

namespace arch {
stack::stack(std::uintptr_t t) : m_top(t) {}

stack stack::new_kernel() {
  char *cPtr = new char[KERNEL_STACK_PAGES * mm::PAGE_SIZE];
  assert(cPtr && "failure to allocate a kernel stack is fatal");

  std::uintptr_t ptr = reinterpret_cast<std::uintptr_t>(cPtr);
  return stack(ptr);
}

// allocate a new user task
std::expected<stack, stack::error_code> stack::new_user() {
  char *cPtr = new char[USER_STACK_PAGES * mm::PAGE_SIZE];
  if (!cPtr)
    return std::unexpected<error_code>(error_code::no_memory);

  std::uintptr_t ptr = reinterpret_cast<std::uintptr_t>(cPtr);
  return stack(ptr);
}
} // namespace arch

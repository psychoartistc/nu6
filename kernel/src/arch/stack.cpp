#include <arch/stack.hpp>
#include <mm/common.hpp>
#include <mm/stl.hpp>

namespace arch {
void stack::enter(std::function<void()> fn) const {
  pr_info("entering 0x%p", top());
  stack_enter(
      top(),
      [](void *arg) {
        auto *func = static_cast<std::function<void()> *>(arg);
        if (func && *func)
          (*func)();
      },
      &fn);
}

stack::stack(std::uintptr_t t) : m_top(t) {}

stack stack::new_kernel() {
  char *cPtr = new char[KERNEL_STACK_PAGES * mm::PAGE_SIZE];
  assert(cPtr && "failure to allocate a kernel stack is fatal");

  std::uintptr_t ptr = reinterpret_cast<std::uintptr_t>(cPtr);
  return stack(ptr + KERNEL_STACK_PAGES * mm::PAGE_SIZE);
}

} // namespace arch

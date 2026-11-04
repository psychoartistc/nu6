#include <arch/stack.hpp>
#include <mm/kernel_mm.hpp>
#include <mm/virtual/allocator.hpp>
#include <mm/common.hpp>
#include <mm/stl.hpp>

namespace arch {

void stack::enter(std::function<void()> fn) const {
  // pr_info("entering 0x%p", top());
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
  auto res = ::mm::allocate_virtual(
      ::mm::get_kernel_vma_tree(), ::mm::get_kernel_pt(),
      KERNEL_STACK_PAGES * ::mm::PAGE_SIZE,
      ::mm::protection::kernel | ::mm::protection::write |
          ::mm::protection::read,
      true, false);

  assert(res && "failure to allocate a kernel stack");

  std::uintptr_t ptr = reinterpret_cast<std::uintptr_t>(*res);
  return stack(ptr + KERNEL_STACK_PAGES * ::mm::PAGE_SIZE);
}

} // namespace arch

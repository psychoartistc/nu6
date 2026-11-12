#include <arch/irq/exceptions.hpp>
#include <utility>
#include <utils/panic.hpp>
#include <arch/irq/irq_dispatcher.hpp>
#include <arch/mm/paging.hpp>
#include <proc/process.hpp>
#include <mm/virtual/allocator.hpp>

namespace arch::irq {
void exceptions::handle_page_fault() {
  namespace kmm = ::mm;

  std::uintptr_t faulting = arch::mm::read_cr2();
  auto currentProc = proc::process::current();
  if (!currentProc) {
    // we are faulting in kernel space
    auto &tree = kmm::get_kernel_vma_tree();
    kmm::vma *found = tree.find(faulting);
    if (!found)
      utils::panic("page fault in kernel mode at 0x%p", faulting);
    else {
      auto res = kmm::map_vma(kmm::get_kernel_pt(), *found, found->protect,
                              found->bypass_wxorx);
      if (res)
        pr_info("demand paged");
      else
        pr_error("failed to demand page!");
    }
    return;
  }

  auto &addrSpace = currentProc->address_space();
  auto &tree = addrSpace.vma_tree();
  auto &pt = addrSpace.page_table();

  kmm::vma *found = tree.find(faulting);
  if (!found)
    currentProc->on_exception(CPU_EXCEPT_PAGE_FAULT);
  else
    auto res = kmm::map_vma(pt, *found, found->protect, found->bypass_wxorx);
}

void exceptions::install() {
  for (std::size_t i = 0; i < EXCEPTION_NUM; i++) {
    irq_dispatcher::set(
        static_cast<std::uint8_t>(i), [](interrupt_frame *frame) {
          std::uint64_t num = frame->int_no;
          if (num == CPU_EXCEPT_PAGE_FAULT) {
            handle_page_fault();
            return;
          }

          utils::panic("cpu exception: %s", EXCEPTION_NAMES[num].data());
          std::unreachable();
        });
  }
}
} // namespace arch::irq

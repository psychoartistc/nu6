#include <arch/smp/core.hpp>
#include <proc/process.hpp>
#include <utils/init_phase.hpp>
#include <arch/stack.hpp>
#include <mm/stl.hpp>
#include <arch/irq/irq_dispatcher.hpp>
#include <arch/io/msr.hpp>
#include <arch/irq/exceptions.hpp>
#include <std/vendor/veque.hpp>

namespace arch::smp {
void core::set_current() {
  io::wrmsr(MSR_GS_BASE, reinterpret_cast<std::uint64_t>(this));
}

core &core::current() {
  return *reinterpret_cast<core *>(io::rdmsr(MSR_GS_BASE));
}

void core::on_launch(bool bsp) {
  // pr_info("core %zu on launch", m_index);
  //  wait until allocators are initialized
  utils::init_phase::wait_for(utils::e_init_states::allocator_ready);
  // pr_info("after allocator ready");
  ::mm::get_kernel_pt().switch_to();
  // all these stacks should never be
  // deleted when our kernel is ran
  m_stack = stack::new_kernel();

  m_tss.initialize();
  m_tss.set_ring0_stack(m_stack.top());

  for (std::size_t i = 0; i < ist::STACK_COUNT; ++i) {
    class stack istStack = stack::new_kernel();
    m_ist.set(i, istStack.top());
  }

  m_ist.install(m_tss);
  m_gdt.initialize(reinterpret_cast<std::uint64_t>(&m_tss.raw()),
                   sizeof(arch::tss::entry) - 1);

  m_gdt.reload();

  m_tss.load(arch::gdt::TSS_SELECTOR);

  if (bsp) {
    idt::initialize();
    irq::irq_dispatcher::install_handlers();

    irq::exceptions::install();
    idt::reload();
  }

  pr_info("core %zu loaded", m_index);

  m_stack.enter([]() {
    idt::reload();
    irq_enable();

    for (;;) {
      auto *d = new int(4);
      pr_info("%d 0x%p", *d, d);
      delete d;
    }

    arch::hcf();
  });
}
} // namespace arch::smp

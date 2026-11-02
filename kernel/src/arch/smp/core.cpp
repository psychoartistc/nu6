#include <arch/smp/core.hpp>
#include <utils/init_phase.hpp>
#include <arch/stack.hpp>
#include <arch/irq/irq_dispatcher.hpp>
#include <arch/io/msr.hpp>

namespace arch::smp {
void core::set_current() {
  io::wrmsr(MSR_GS_BASE, reinterpret_cast<std::uint64_t>(this));
}

core &core::current() {
  return *reinterpret_cast<core *>(io::rdmsr(MSR_GS_BASE));
}

void core::on_launch(bool bsp) {
  // wait until allocators are initialized
  utils::init_phase::wait_for(utils::e_init_states::allocator_ready);
  // all these stacks should never be
  // deleted when our kernel is ran
  m_stack = stack::new_kernel();

  m_tss.initialize();
  m_tss.set_ring0_stack(m_stack.top());

  for (std::size_t i = 0; i < ist::STACK_COUNT; ++i) {
    stack istStack = stack::new_kernel();
    m_ist.set(i, istStack.top());
  }

  m_ist.install(m_tss);
  m_gdt.initialize_and_load(reinterpret_cast<std::uint64_t>(&m_tss.raw()),
                            sizeof(arch::tss::entry) - 1);

  m_tss.load(arch::gdt::TSS_SELECTOR);

  if (bsp) {
    idt::initialize_and_load();
    irq::irq_dispatcher::install_handlers();
  }
}
} // namespace arch::smp

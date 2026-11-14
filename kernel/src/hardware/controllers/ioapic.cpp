#include <hardware/controllers/ioapic.hpp>
#include <arch/irq/common.hpp>
#include <vendor/magic_enum.hpp>
#include <mm/kernel_mm.hpp>

namespace hardware::ctl {

std::uint32_t ioapic::read_reg_locked(std::uint8_t reg) {
  m_mmio.write<std::uint32_t>(REGSEL, reg);

  std::atomic_signal_fence(std::memory_order_seq_cst);

  std::uint32_t val = m_mmio.read<std::uint32_t>(WINDOW);

  std::atomic_signal_fence(std::memory_order_seq_cst);

  return val;
}

void ioapic::write_reg(std::uint8_t reg, std::uint32_t val) {}

ioapic::ioapic(acpi::madt_ioapic entry) : m_madtEntry(entry) {
  auto &tree = mm::get_kernel_vma_tree();
  auto &pt = mm::get_kernel_pt();

  mm::vma *res = tree.allocate(4096);
  if (!res) {
    s_logger.warn("failed to allocate vma for ioapic %zu",
                  m_madtEntry.io_apic_id);
    return;
  }

  auto res2 = pt.map(entry.io_apic_address, res->address, utils::mmio::PROT);
  if (!res2) {
    s_logger.warn("failed to map ioapic physical 0x%p to virtual 0x%p: %s",
                  entry.io_apic_address, res->address,
                  magic_enum::enum_name(res2.error()));
    return;
  }

  m_mmio.change_address(res->address);
  m_destApicId = arch::irq::read_boot_apic();

  std::lock_guard<std::spinlock_mutex> lock(m_mtx);

  std::uint32_t
}

std::vector<ioapic> &ioapic::all() { return s_ioapics; }

std::vector<ioapic> ioapic::s_ioapics{};
logging::named_logger ioapic::s_logger{"ioapic"};
} // namespace hardware::ctl

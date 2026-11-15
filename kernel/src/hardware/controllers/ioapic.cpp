#include <hardware/controllers/ioapic.hpp>
#include <arch/irq/common.hpp>
#include <vendor/magic_enum.hpp>
#include <mm/kernel_mm.hpp>

namespace hardware::ctl {

std::uint32_t ioapic::read(std::uint8_t reg) {
  m_mmio.write<std::uint32_t>(REGSEL, reg);

  std::atomic_signal_fence(std::memory_order_seq_cst);

  std::uint32_t val = m_mmio.read<std::uint32_t>(WINDOW);

  std::atomic_signal_fence(std::memory_order_seq_cst);

  return val;
}

void ioapic::write(std::uint8_t reg, std::uint32_t val) {
  m_mmio.write<std::uint32_t>(REGSEL, reg);

  std::atomic_signal_fence(std::memory_order_seq_cst);

  m_mmio.write<std::uint32_t>(WINDOW, val);

  std::atomic_signal_fence(std::memory_order_seq_cst);
}

ioapic::ioapic(acpi::madt_ioapic entry) : m_madtEntry(entry) {
  auto &tree = mm::get_kernel_vma_tree();
  auto &pt = mm::get_kernel_pt();

  mm::vma *res = tree.allocate(4096);
  if (!res) {
    s_logger.warn("failed to allocate vma for ioapic %zu",
                  m_madtEntry.io_apic_id);

    m_errorCode = error_code::no_virtual_mem;
    return;
  }

  auto res2 = pt.map(entry.io_apic_address, res->address, utils::mmio::PROT);
  if (!res2) {
    s_logger.warn("failed to map ioapic physical 0x%p to virtual 0x%p: %s",
                  entry.io_apic_address, res->address,
                  magic_enum::enum_name(res2.error()));

    m_errorCode = error_code::map_error;
    return;
  }

  m_mmio.change_address(res->address);
  m_destApicId = arch::irq::read_boot_apic();

  std::lock_guard<std::spinlock_mutex> lock(m_mtx);

  std::uint32_t version = read(REG_VERSION);
  if (version == 0xFFFFFFFFU || (version & 0xFFU) == 0) {
    s_logger.warn("invalid version register: 0x%p", version);
    m_errorCode = error_code::bad_version_register;
    return;
  }

  std::uint32_t maxRedir = (version >> 16) & 0xFFU;
  std::uint32_t count = maxRedir + 1;

  if (count > MAX_REDIR_COUNT) {
    s_logger.warn("unsupported redirection count: %zu", count);
    m_errorCode = error_code::bad_redir_count;
    return;
  }

  m_redirCount = count;

  for (std::uint32_t i = 0; i < m_redirCount; ++i) {
    std::uint8_t rlow = static_cast<std::uint8_t>(REDIR_BASE + 2U * i);

    std::uint32_t low = read(rlow);

    write(rlow, low | REDIR_MASK);
  }

  m_bInitialized = true;
}

std::expected<void, ioapic::error_code> ioapic::route_irq(std::uint32_t route,
                                                          std::uint8_t vec) {
  if (!initialized())
    return std::unexpected<error_code>(error_code::not_initialized);

  // TODO: fix the next 3 ifs with a goto
  if (vec < 0x20 || vec == 0xFF)
    return std::unexpected<error_code>(error_code::bad_arg);

  if (static_cast<std::uintptr_t>(route) <
      m_madtEntry.global_system_interrupt_base)
    return std::unexpected<error_code>(error_code::bad_arg);

  std::uintptr_t offset = static_cast<std::uintptr_t>(route) -
                          m_madtEntry.global_system_interrupt_base;
  if (offset >= m_redirCount)
    return std::unexpected<error_code>(error_code::bad_arg);

  std::uint8_t lowr = static_cast<std::uint8_t>(
      REDIR_BASE + 2U * static_cast<std::uint32_t>(offset));
  std::uint8_t highr = lowr + 1;

  std::uint32_t low = static_cast<std::uint32_t>(vec) | REDIR_MASK;
  std::uint32_t high = static_cast<std::uint32_t>(m_destApicId) << 24;

  {
    std::lock_guard<std::spinlock_mutex> lock(m_mtx);

    write(lowr, low);
    write(highr, high);

    write(lowr, static_cast<std::uint32_t>(vec));
  }

  s_logger.info("[%zu] routed gsi %d to vector %d", m_madtEntry.io_apic_id,
                route, vec);

  return {};
}

std::vector<std::unique_ptr<ioapic>> &ioapic::all() { return s_ioapics; }

std::vector<std::unique_ptr<ioapic>> ioapic::s_ioapics{};
logging::named_logger ioapic::s_logger{"ioapic"};
} // namespace hardware::ctl

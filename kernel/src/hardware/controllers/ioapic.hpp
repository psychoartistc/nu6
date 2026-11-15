#pragma once

#include <utils/mmio.hpp>
#include <std/vendor/veque.hpp>
#include <cstddef>
#include <expected>
#include <cstdint>
#include <hardware/acpi/madt.hpp>
#include <std/spinlock.hpp>

namespace hardware::ctl {
class ioapic {
private:
  enum class error_code {
    ok,
    no_virtual_mem,
    map_error,
    bad_version_register,
    bad_redir_count,
    bad_arg,
    not_initialized
  };

private:
  static inline constexpr std::uint64_t REGSEL = 0x0;
  static inline constexpr std::uint64_t WINDOW = 0x10;

  static inline constexpr std::uint64_t REG_ID = 0x00;
  static inline constexpr std::uint64_t REG_VERSION = 0x01;

  static inline constexpr std::uint64_t REDIR_BASE = 0x10;
  static inline constexpr std::uint64_t REDIR_MASK = (1U << 16);
  static inline constexpr std::uint64_t MAX_REDIR_COUNT = 120U;

private:
  static std::vector<std::unique_ptr<ioapic>> s_ioapics;
  static logging::named_logger s_logger;

  acpi::madt_ioapic m_madtEntry{};
  utils::mmio m_mmio{0x0};

  error_code m_errorCode{error_code::ok};
  std::uint32_t m_redirCount{0x0};
  std::uint8_t m_destApicId{0x0};

  std::spinlock_mutex m_mtx;
  bool m_bInitialized = false;

private:
  std::uint32_t read(std::uint8_t reg);
  void write(std::uint8_t reg, std::uint32_t val);

public:
  // ioapic's are discovered through the madt table
  ioapic(acpi::madt_ioapic m);

  const acpi::madt_ioapic &madt_entry() const { return m_madtEntry; }

  std::expected<void, error_code> route_irq(std::uint32_t route,
                                            std::uint8_t vec);

  bool initialized() const { return m_bInitialized; }

  static std::vector<std::unique_ptr<ioapic>> &all();

  error_code err_code() const { return m_errorCode; }
};
} // namespace hardware::ctl

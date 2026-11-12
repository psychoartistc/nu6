#pragma once

#include <hardware/acpi/table_parser.hpp>
#include <logging/logging.hpp>

namespace hardware::acpi {

struct [[gnu::packed]] madt_table {
  table_header header;
  std::uint32_t local_apic_address;
  std::uint32_t flags;
  std::uint8_t entries[];
};

struct [[gnu::packed]] madt_entry_header {
  std::uint8_t type;
  std::uint8_t length;
};

struct [[gnu::packed]] madt_ioapic {
  madt_entry_header header;
  std::uint8_t io_apic_id;
  std::uint8_t reserved;
  std::uint32_t io_apic_address;
  std::uint32_t global_system_interrupt_base;
};

class madt_parser : public table_parser {
private:
  static inline constexpr std::uint32_t IOAPIC_CODE = 1;

private:
  logging::named_logger m_logger{"madt_parser"};

public:
  ~madt_parser() = default;

  inline constexpr std::string_view table_name() const override {
    return "APIC";
  }

  bool parse(table_header *header) override;
};
} // namespace hardware::acpi

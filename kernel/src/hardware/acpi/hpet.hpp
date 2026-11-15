#pragma once

#include <logging/logging.hpp>
#include <hardware/acpi/table_parser.hpp>

namespace hardware::acpi {
struct [[gnu::packed]] hpet_table {
  table_header header;
  std::uint32_t event_timer_block_id;
  gas_contents base_address;
  std::uint8_t hpet_number;
  std::uint16_t minimum_tick;
  std::uint8_t page_protection;
};

class hpet_parser : public table_parser {
private:
  logging::named_logger m_logger{"acpi_hpet"};

public:
  ~hpet_parser() = default;

  inline constexpr std::string_view table_name() const override {
    return "HPET";
  }

  bool parse(table_header *header) override;
};
} // namespace hardware::acpi

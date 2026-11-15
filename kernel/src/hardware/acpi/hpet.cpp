#include <hardware/acpi/hpet.hpp>
#include <modules/api.hpp>
#include <hardware/acpi/table_manager.hpp>
#include <proc/timers/hpet.hpp>

namespace hardware::acpi {
static inline constexpr int GAS_SYSTEM_MEMORY = 0;

bool hpet_parser::parse(table_header *header) {
  hpet_table *tbl = reinterpret_cast<hpet_table *>(header);

  if (tbl->base_address.address_space_id != GAS_SYSTEM_MEMORY) {
    m_logger.warn("hpet is not memory mapped");
    return false;
  }

  proc::timers::hpet::set_physical_base(tbl->base_address.address);
  return true;
}

static bool hpet_parser_entry() {
  table_manager::add_parser<hpet_parser>();
  return true;
}
} // namespace hardware::acpi

REGISTER_MODULE(madt_table_parser, "hpet acpi table parser",
                "parses the hpet acpi table", "1.0.0",
                module::MDLLICENSE_DEFAULT,
                module::module_init_order::acpi_parser_populate,
                hardware::acpi::hpet_parser_entry, NULL);

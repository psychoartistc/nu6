#include <hardware/acpi/madt.hpp>
#include <modules/api.hpp>
#include <hardware/acpi/table_manager.hpp>
#include <hardware/controllers/pic8259.hpp>
#include <hardware/controllers/ioapic.hpp>
#include <hardware/controllers/lapic.hpp>

namespace hardware::acpi {
bool madt_parser::parse(table_header *hdr) {
  madt_table *table = reinterpret_cast<madt_table *>(hdr);
  ctl::lapic::initialize(table->local_apic_address);

  std::uintptr_t madtPtr = reinterpret_cast<std::uintptr_t>(table);

  std::uintptr_t end = madtPtr + table->header.length;
  std::uintptr_t current = reinterpret_cast<std::uintptr_t>(table->entries);

  while (current < end) {
    madt_entry_header *entry = reinterpret_cast<madt_entry_header *>(current);

    if (entry->type == IOAPIC_CODE)
      ctl::ioapic::all().emplace_back(*reinterpret_cast<madt_ioapic *>(entry));

    if (entry->length == 0)
      break;

    current += entry->length;
  }

  assert(ctl::ioapic::all().size() > 0 && "no ioapics found, fatal");

  ctl::pic8259::disable();

  m_logger.info("added %zu ioapic(s)", ctl::ioapic::all().size());

  return true;
}

static bool madt_parse_entry() {
  table_manager::add_parser<madt_parser>();
  return true;
}

} // namespace hardware::acpi

REGISTER_MODULE(madt_table_parser, "madt acpi table parser",
                "parses the madt acpi table", "1.0.0",
                module::MDLLICENSE_DEFAULT,
                module::module_init_order::acpi_parser_populate,
                hardware::acpi::madt_parse_entry, NULL);

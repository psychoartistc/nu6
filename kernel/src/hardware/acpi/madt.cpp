#include <hardware/acpi/madt.hpp>

namespace hardware::acpi {
bool madt_parser::parse(table_header *hdr) {
  madt_table *table = reinterpret_cast<madt_table *>(hdr);
}

} // namespace hardware::acpi

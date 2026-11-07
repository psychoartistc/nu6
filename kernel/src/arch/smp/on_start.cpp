#include <arch/smp/core.hpp>
#include <hardware/acpi/table_manager.hpp>

namespace arch {
namespace smp {
void core::on_start() {
  if (m_bBsp)
    hardware::acpi::table_manager::parse_all();
}
} // namespace smp
} // namespace arch

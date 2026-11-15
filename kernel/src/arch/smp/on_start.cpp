#include <arch/smp/core.hpp>
#include <proc/timer_source.hpp>
#include <hardware/acpi/table_manager.hpp>

namespace arch {
namespace smp {
void core::on_start() {
  if (m_bBsp) {
    hardware::acpi::table_manager::parse_all();

    irq_enable();
    proc::timer_source::initialize_sources();

    proc::timer_source::current()->start(1000000000);
  }
}
} // namespace smp
} // namespace arch

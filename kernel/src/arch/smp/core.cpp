#include <arch/smp/core.hpp>
#include <arch/io/msr.hpp>

namespace arch::smp {
void core::set_current() {
  io::wrmsr(MSR_GS_BASE, reinterpret_cast<std::uint64_t>(this));
}

core &core::current() {
  return *reinterpret_cast<core *>(io::rdmsr(MSR_GS_BASE));
}
} // namespace arch::smp

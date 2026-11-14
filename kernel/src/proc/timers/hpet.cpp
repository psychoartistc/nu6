#include <proc/timers/hpet.hpp>
#include <utils/math.hpp>
#include <std/stubs/uint128.hpp>

namespace proc::timers {
std::uint64_t hpet::read_counter() {
  if (m_bCounter64)
    return m_mmio.read<std::uint64_t>(MAIN_COUNTER);

  return m_mmio.read<std::uint32_t>(MAIN_COUNTER);
}

std::expected<std::uint64_t, hpet::error_code>
hpet::ns_to_ticks(std::uint64_t ns) {
  std::uint128_t scaled = (std::uint128_t)ns * 1000000ULL;
  scaled = utils::div_ceil<std::uint128_t>(scaled, m_periodFs);

  if (scaled < 1)
    scaled = 1;

  scaled += MIN_LEAD_TICKS;

  if (m_bUse64Compare) {
    if (scaled > 0x7FFFFFFFFFFFFFFFULL)
      return std::unexpected<error_code>(error_code::overflow);
  } else {
    if (scaled > 0x7FFFFFFFULL)
      return std::unexpected<error_code>(error_code::overflow);
  }

  return static_cast<std::uint64_t>(scaled);
}

} // namespace proc::timers

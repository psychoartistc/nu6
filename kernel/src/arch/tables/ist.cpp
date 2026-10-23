#include <arch/tables/ist.hpp>
#include <arch/tables/tss.hpp>

namespace arch {

std::uint64_t ist::top(std::size_t index) const {
  if (index >= STACK_COUNT) {
    return 0;
  }

  return m_stack_tops[index];
}

void ist::install(tss &target) const {
  for (std::size_t i = 0; i < STACK_COUNT; ++i) {
    target.set_ist(static_cast<std::uint8_t>(i + 1), top(i));
  }
}

} // namespace arch

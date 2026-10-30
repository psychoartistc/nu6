#pragma once

#include <cstdint>

namespace arch::smp {
class core {
private:
  static inline constexpr std::uint64_t MSR_GS_BASE = 0xC0000101;

private:
  std::uint64_t m_index{0};
  std::uint64_t m_lapicId{0};

public:
  core(std::uint64_t idx, std::uint64_t lapic)
      : m_index(idx), m_lapicId(lapic) {}

  core() = default;

  std::uint64_t lapic_id() const { return m_lapicId; }
  std::uint64_t index() const { return m_index; }

  void set_current();

  static core &current();
};

} // namespace arch::smp

#pragma once

#include <cstddef>
#include <cstdint>
#include <logging/logging.hpp>

namespace arch {

class tss {
public:
  struct entry {
    std::uint32_t reserved0;

    std::uint64_t rsp[3];

    std::uint64_t reserved1;

    std::uint64_t ist[7];

    std::uint64_t reserved2;
    std::uint16_t reserved3;

    std::uint16_t io_map_base;
  } __PACKED;

  static constexpr std::size_t ist_count = 7;

private:
  entry m_tss{};
  logging::named_logger m_logger{"tss"};

public:
  void initialize();

  void set_ring0_stack(std::uint64_t stack_top);

  void set_ist(std::uint8_t ist_index, std::uint64_t stack_top);

  void load(std::uint16_t selector);

  [[nodiscard]]
  const entry &raw() const {
    return m_tss;
  }

  [[nodiscard]]
  entry &raw() {
    return m_tss;
  }
};

} // namespace arch

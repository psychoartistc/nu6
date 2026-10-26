#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <logging/logging.hpp>

namespace arch {

class gdt {
public:
  struct [[gnu::packed]] gdt_entry {
    std::uint16_t limit_low;
    std::uint16_t base_low;
    std::uint8_t base_middle;
    std::uint8_t access;
    std::uint8_t granularity;
    std::uint8_t base_high;
  };

  struct [[gnu::packed]] gdt_ptr {
    std::uint16_t limit;
    std::uint64_t base;
  };

  static constexpr std::uint16_t KERNEL_CODE_SELECTOR = 0x08;
  static constexpr std::uint16_t KERNEL_DATA_SELECTOR = 0x10;

  static constexpr std::uint16_t USER_CODE_SELECTOR = 0x33;
  static constexpr std::uint16_t USER_DATA_SELECTOR = 0x2B;

  static constexpr std::uint16_t TSS_SELECTOR = 0x18;

  static constexpr std::size_t GDT_ENTRIES = 7;
  static constexpr std::size_t TSS_INDEX = 3;

private:
  std::array<gdt_entry, 7> m_gdt{};
  gdt_ptr m_gdtPtr{};
  logging::named_logger m_logger{"gdt"};

public:
  void set_gate(std::size_t num, std::uint32_t base, std::uint32_t limit,
                std::uint8_t access, std::uint8_t gran);

  void set_tss_gate(std::size_t num, std::uint64_t base, std::uint32_t limit);

  void initialize_and_load(std::uint64_t tss_base, std::uint32_t tss_limit);

  void reload();
};

} // namespace arch

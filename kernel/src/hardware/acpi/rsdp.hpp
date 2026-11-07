#pragma once

#include <cstddef>
#include <cstdint>
#include <array>

namespace hardware::acpi {
class [[gnu::packed]] rsdp {
private:
  static inline constexpr std::size_t RSDP_SIGNATURE_SIZE = 8;
  static inline constexpr std::array<char, RSDP_SIGNATURE_SIZE> RSDP_SIGNATURE =
      {'R', 'S', 'D', ' ', 'P', 'T', 'R', ' '};

public:
  enum class validate_fail_reason {
    bad_signature,
    bad_checksum_rsdp,
    bad_checksum_xsdp,
    ok
  };

public:
  std::array<char, RSDP_SIGNATURE_SIZE> signature;
  std::uint8_t checksum;
  std::array<char, 6> oem_id;
  std::uint8_t revision;
  std::uint32_t rsdt_address;

  std::uint32_t length;
  std::uint64_t xsdt_address;
  std::uint8_t extended_checksum;
  std::array<std::uint8_t, 3> reserved;

public:
  // https://wiki.osdev.org/RSDP#Validating
  validate_fail_reason validate();
};
} // namespace hardware::acpi

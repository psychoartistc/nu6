#include <hardware/acpi/rsdp.hpp>
#include <span>
#include <std/memory.hpp>
#include <logging/logging.hpp>

namespace hardware::acpi {
static bool checksum_bytes(std::span<std::uint8_t> l) {
  std::uint8_t sum = 0;
  for (std::uint8_t c : l)
    sum += c;

  return sum == 0;
}

rsdp::validate_fail_reason rsdp::validate() {
  // check signature
  for (std::size_t i = 0; i < RSDP_SIGNATURE_SIZE; i++) {
    if (signature[i] != RSDP_SIGNATURE[i])
      return validate_fail_reason::bad_signature;
  }

  std::span<std::uint8_t> rsdp(reinterpret_cast<std::uint8_t *>(this), 20);
  bool chksum = checksum_bytes(rsdp);

  if (chksum && revision >= 2) {
    rsdp =
        std::span<std::uint8_t>(reinterpret_cast<std::uint8_t *>(this), length);

    chksum = checksum_bytes(rsdp);
    if (!chksum)
      return validate_fail_reason::bad_checksum_xsdp;
  } else if (!chksum)
    return validate_fail_reason::bad_checksum_rsdp;

  return validate_fail_reason::ok;
}
} // namespace hardware::acpi

#pragma once

#include <cstdint>
#include <cstddef>

namespace hardware {
namespace acpi {
struct [[gnu::packed]] table_header {
  char signature[4];
  std::uint32_t length;
  std::uint8_t revision;
  std::uint8_t checksum;
  char oem_id[6];
  char oem_table_id[8];
  std::uint32_t oem_revision;
  std::uint32_t creator_id;
  std::uint32_t creator_revision;
};

struct [[gnu::packed]] rsdp_contents {
  char signature[8];
  std::uint8_t checksum;
  char oem_id[6];
  std::uint8_t revision;
  std::uint32_t rsdt_address;
  std::uint32_t length;
  std::uint64_t xsdt_address;
  std::uint8_t extended_checksum;
  std::uint8_t reserved[3];
};

struct [[gnu::packed]] rsdt_contents {
  table_header header;
};

struct [[gnu::packed]] xsdt_contents {
  table_header header;
};

struct [[gnu::packed]] gas_contents {
  std::uint8_t address_space_id;
  std::uint8_t register_bit_width;
  std::uint8_t register_bit_offset;
  std::uint8_t access_size;
  std::uint64_t address;
};

} // namespace acpi
} // namespace hardware

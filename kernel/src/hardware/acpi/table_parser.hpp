#pragma once

#include <hardware/acpi/common.hpp>
#include <string_view>

namespace hardware {
namespace acpi {
// abstract type for a parser of some table
class table_parser {
public:
  virtual ~table_parser() {}

  virtual inline constexpr std::string_view table_name() const = 0;

  virtual bool parse(table_header *header) = 0;
};
} // namespace acpi
} // namespace hardware

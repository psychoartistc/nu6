#pragma once

#include <string_view>

namespace hardware {
namespace acpi {
// abstract type for a parser of some table
class table_parser {
public:
  virtual inline constexpr std::string_view table_name() const = 0;
};
} // namespace acpi
} // namespace hardware

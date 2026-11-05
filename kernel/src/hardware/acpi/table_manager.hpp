#pragma once

#include <logging/logging.hpp>
#include <std/vendor/veque.hpp>
#include <memory>
#include <hardware/acpi/table_parser.hpp>
#include <std/stubs/make_unique.hpp>

namespace hardware {
namespace acpi {
class table_manager {
private:
  static std::vector<std::unique_ptr<table_parser>> s_parsers;
  static logging::named_logger s_logger;

private:
  static void dispatch_parse(table_header *tbl);
  static void parse_rsdt(rsdp_contents *rsdp);
  static void parse_xsdt(rsdp_contents *rsdp);

public:
  template <std::derived_from<table_parser> T> static void add_parser() {
    std::unique_ptr<T> ty = std::make_unique<T>();
    s_parsers.push_back(std::move(ty));
  }

  static void parse_all();
};
} // namespace acpi
} // namespace hardware

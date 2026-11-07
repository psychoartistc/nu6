#include <hardware/acpi/table_manager.hpp>
#include <vendor/magic_enum.hpp>
#include <limine/requests.hpp>
#include <mm/common.hpp>

namespace hardware {
namespace acpi {
std::vector<std::unique_ptr<table_parser>> table_manager::s_parsers{};
logging::named_logger table_manager::s_logger{"acpi"};

void table_manager::dispatch_parse(table_header *hdr) {
  std::string_view signature(hdr->signature, 4);
  for (const std::unique_ptr<table_parser> &parser : s_parsers) {
    if (signature != parser->table_name())
      continue;

    s_logger.info("parsing %.4s", signature.data());
    bool success = parser->parse(hdr);
    if (!success)
      s_logger.error("failed to parse acpi table \'%.4s\'!", signature.data());
  }
}

void table_manager::parse_xsdt(rsdp *rsdp) {
  xsdt_contents *xsdt =
      reinterpret_cast<xsdt_contents *>(mm::p2v(rsdp->xsdt_address));

  std::size_t entry_count =
      (xsdt->header.length - sizeof(table_header)) / sizeof(std::uint64_t);

  auto *tables = reinterpret_cast<std::uint64_t *>(
      reinterpret_cast<std::uintptr_t>(xsdt) + sizeof(table_header));

  for (std::size_t i = 0; i < entry_count; i++) {
    table_header *tb = reinterpret_cast<table_header *>(mm::p2v(tables[i]));

    dispatch_parse(tb);
  }
}

void table_manager::parse_rsdt(rsdp *rsdp) {
  rsdt_contents *rsdt =
      reinterpret_cast<rsdt_contents *>(mm::p2v(rsdp->rsdt_address));

  std::uintptr_t rsdt_virt = reinterpret_cast<std::uintptr_t>(rsdt);

  std::size_t entry_count =
      (rsdt->header.length - sizeof(table_header)) / sizeof(std::uint32_t);

  auto *tbls =
      reinterpret_cast<std::uint32_t *>(rsdt_virt + sizeof(table_header));

  for (std::size_t i = 0; i < entry_count; i++) {
    table_header *tb = reinterpret_cast<table_header *>(mm::p2v(tbls[i]));

    dispatch_parse(tb);
  }
}

void table_manager::parse_all() {
  auto *resp = limine::rsdp_request.response;
  if (!resp)
    return;

  rsdp *rsdp =
      reinterpret_cast<class rsdp *>(limine::rsdp_request.response->address);

  auto r = rsdp->validate();
  if (r != rsdp::validate_fail_reason::ok) {
    utils::panic("bad rsdp signature! reason: %s", magic_enum::enum_name(r));
  }

  s_logger.info("using acpi v%d", rsdp->revision);
  bool xsdt = (rsdp->revision >= 2 && rsdp->xsdt_address != 0);

  if (xsdt)
    parse_xsdt(rsdp);
  else
    parse_rsdt(rsdp);
}
} // namespace acpi
} // namespace hardware

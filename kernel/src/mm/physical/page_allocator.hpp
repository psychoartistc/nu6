#pragma once

#include <cstdint>
#include <expected>
#include <logging/logging.hpp>

namespace mm {
class page_allocator {
public:
  enum class error_code { no_mem, unimplemented, bad_arg };

private:
  static logging::named_logger s_logger;

public:
  static bool initialize();

  static std::expected<std::uintptr_t, error_code>
  allocate(std::uint64_t count);

  static bool free(std::uintptr_t at, std::uint64_t count);

  static bool reclaim_acpi();
  static bool reclaim_bootloader();
};
} // namespace mm

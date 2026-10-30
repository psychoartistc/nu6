#pragma once

#include <limine.h>
#include <array>
#include <atomic>
#include <arch/smp/core.hpp>
#include <logging/logging.hpp>

namespace arch::smp {
static inline constexpr std::uint64_t MAX_CORES = 256;

class core_manager {
private:
  static std::array<core, MAX_CORES> s_cores;
  static logging::named_logger s_logger;
  static std::atomic<std::uint64_t> s_coresOnline;

  void ap_entry(limine_mp_info *m);

public:
  static void launch_all();

  static std::uint64_t core_count();
};
} // namespace arch::smp

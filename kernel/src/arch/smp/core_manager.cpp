#include <arch/smp/core_manager.hpp>
#include <utility>
#include <limine/requests.hpp>

namespace arch::smp {
std::array<core, MAX_CORES> core_manager::s_cores{};
std::atomic<std::uint64_t> core_manager::s_coresOnline{0};

logging::named_logger core_manager::s_logger{"core_manager"};

void core_manager::ap_entry(limine_mp_info *info) {
  std::uint64_t idx = info->extra_argument;
  s_cores[idx].set_current();

  s_coresOnline.fetch_add(1, std::memory_order_release);

  std::unreachable();
  arch::hcf();
}

void core_manager::launch_all() {
  auto *resp = limine::mp_request.response;
  if (!resp) {
    std::construct_at(&s_cores[0], 0, 0);

    s_cores[0].set_current();

    s_coresOnline.fetch_add(1, std::memory_order_release);
    return;
  }

  std::uint32_t bspId = resp->bsp_lapic_id;
  std::uint64_t count = core_count();

  for (std::uint64_t i = 0; i < count; i++) {
    auto *info = resp->cpus[i];

    std::construct_at(&s_cores[i], i, info->lapic_id);
    if (info->lapic_id == bspId) {
      s_cores[i].set_current();
      s_coresOnline.fetch_add(1, std::memory_order_release);

      continue;
    }

    info->extra_argument = i;

    std::atomic_ref<limine_goto_address> addr(info->goto_address);
    addr.store(ap_entry, std::memory_order_seq_cst);
  }

  while (s_coresOnline.load(std::memory_order_acquire) < count)
    pause();

  s_logger.info("all %zu cores launched", count);
}

static std::uint64_t core_count_impl() {
  if (!limine::mp_request.response)
    return 1;

  return std::min(limine::mp_request.response->cpu_count, MAX_CORES);
}

std::uint64_t core_manager::core_count() {
  static std::uint64_t cache = core_count_impl();
  return cache;
}
} // namespace arch::smp

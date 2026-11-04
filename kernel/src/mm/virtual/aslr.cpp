#include <mm/virtual/aslr.hpp>
#include <mm/common.hpp>

namespace mm {

std::uint64_t aslr_manager::random_u64() {
  std::array<std::uint8_t, sizeof(std::uint64_t)> bytes{};

  s_rng.fill_bytes(std::span<std::uint8_t>(bytes));

  std::uint64_t value = 0;

  for (std::size_t i = 0; i < bytes.size(); ++i) {
    value |= static_cast<std::uint64_t>(bytes[i]) << (i * 8);
  }

  return value;
}

std::uintptr_t aslr_manager::random_offset(std::uintptr_t max_offset) {
  std::uint64_t slots = (max_offset / PAGE_SIZE) + 1;

  std::uint64_t threshold = -slots % slots;

  std::uint64_t value;

  do {
    value = random_u64();
  } while (value < threshold);

  return (value % slots) * PAGE_SIZE;
}

void aslr_manager::initialize() {
  std::array<std::uint64_t, utils::seed_generator::SEED_SIZE> seed;
  utils::seed_generator::generate_seed(seed);

  s_rng.seed(seed);
}

std::pair<std::uintptr_t, std::uintptr_t> aslr_manager::generate_kernel() {
  constexpr auto size = KERNEL_VMA_SIZE;

  constexpr auto window_size = KERNEL_VMA_END - KERNEL_VMA_START;

  static_assert(window_size >= size);

  constexpr auto max_offset = window_size - size;

  auto start = KERNEL_VMA_START + random_offset(max_offset);

  return {start, start + size};
}

std::pair<std::uintptr_t, std::uintptr_t> aslr_manager::generate_user() {
  constexpr auto size = USER_VMA_SIZE;

  constexpr auto window_size = USER_VMA_END - USER_VMA_START + 1;

  static_assert(window_size >= size);

  constexpr auto max_offset = window_size - size;

  auto start = USER_VMA_START + random_offset(max_offset);

  return {start, start + size};
}

utils::rng aslr_manager::s_rng{};
} // namespace mm

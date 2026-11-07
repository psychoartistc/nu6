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
  std::uint64_t start = KERNEL_VMA_START + random_offset(MAX_OFFSET);

  return {start, start + KERNEL_VMA_SIZE};
}

std::pair<std::uintptr_t, std::uintptr_t> aslr_manager::generate_user() {
  auto start = USER_VMA_START + random_offset(MAX_OFFSET);

  return {start, start + USER_VMA_SIZE};
}

utils::rng aslr_manager::s_rng{};
} // namespace mm

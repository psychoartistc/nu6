#include <utils/generate_seed.hpp>
#include <utility>
#include <utils/panic.hpp>
#include <limine/requests.hpp>

static std::uint64_t split_mix64(std::uint64_t x) {
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}

static std::uint64_t generate_seed() {
  std::uint64_t seed = 0;

  auto *resp = limine::entropy_request.response;
  if (!resp || resp->value_count == 0) {
    utils::panic("no entropy request provided");
    std::unreachable();
    return 0;
  }

  for (std::size_t i = 0; i < resp->value_count; i++) {
    seed ^= split_mix64(resp->values[i] + i * 0x9e3779b97f4a7c15ULL);

    seed = split_mix64(seed);
  }

  return seed;
}

std::uint64_t rng_seed() {
  static std::uint64_t seed = generate_seed();
  return seed;
}

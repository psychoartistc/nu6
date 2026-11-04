#pragma once

#include <optional>
#include <logging/logging.hpp>
#include <cstddef>
#include <array>
#include <cstdint>
#include <span>

namespace utils {
class rng {
private:
  std::uint32_t m_state[16];

private:
  void rekey();

public:
  void seed(std::span<const std::uint64_t> entropy);

  ~rng();

  static rng from_limine();

  void fill_bytes(std::span<std::uint8_t> out);
};

class seed_generator {
public:
  static inline constexpr std::size_t SEED_SIZE = 8;

private:
  static rng s_rng;
  static logging::named_logger s_logger;

public:
  static void initialize();

  static void generate_seed(std::span<std::uint64_t> at);
};
} // namespace utils

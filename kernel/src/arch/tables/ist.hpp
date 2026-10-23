#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace arch {

class tss;

class ist {
public:
  static constexpr std::size_t STACK_COUNT = 7;

private:
  std::array<std::uint64_t, STACK_COUNT> m_stack_tops{};

public:
  [[nodiscard]]
  std::uint64_t top(std::size_t index) const;

  inline void set(std::size_t index, std::uint64_t stack_top) {
    m_stack_tops[index] = stack_top;
  }

  void install(tss &target) const;
};

} // namespace arch

#pragma once

#include <string_view>
#include <std/turnstile.hpp>

namespace utils {
enum class e_init_states { early, allocator_ready, scheduler_ready };

static inline constexpr std::string_view init_state_str(e_init_states state) {
  switch (state) {
  case e_init_states::early:
    return "early";
  case e_init_states::allocator_ready:
    return "allocator_ready";
  case e_init_states::scheduler_ready:
    return "scheduler_ready";
  default:
    return "unknown";
  };
}

class init_phase {
private:
  static std::turnstile<e_init_states> s_turnstile;

public:
  static void wait_for(e_init_states state);
  static void checkpoint(e_init_states state);
};
} // namespace utils

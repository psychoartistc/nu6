#include <utils/init_phase.hpp>

namespace utils {
std::turnstile<e_init_states> init_phase::s_turnstile{e_init_states::early};

void init_phase::wait_for(e_init_states state) { s_turnstile.wait_for(state); }
void init_phase::checkpoint(e_init_states state) { s_turnstile.set(state); }
} // namespace utils

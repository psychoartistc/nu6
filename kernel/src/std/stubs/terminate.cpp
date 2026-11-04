#include <std/stubs/terminate.hpp>
#include <arch/control.hpp>

void std::terminate() noexcept {
  utils::panic("std::terminate called, this shouldnt happen!");
}

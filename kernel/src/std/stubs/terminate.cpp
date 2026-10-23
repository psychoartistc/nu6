#include <std/stubs/terminate.hpp>
#include <arch/control.hpp>

void std::terminate() { arch::hcf(); }

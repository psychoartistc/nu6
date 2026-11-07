#include <security/stack_protect.hpp>
#include <utils/panic.hpp>

std::uintptr_t __stack_chk_guard = security::STACK_CHK_GUARD;

extern "C" {
void __stack_chk_fail(void) { utils::panic("stack protector failed!"); }
}

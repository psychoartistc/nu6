#include <std/stubs/abort.hpp>

namespace std {
void abort() { utils::panic("std::abort called, this shouldnt happen!"); }
} // namespace std

extern "C" {
void abort() { std::abort(); }
}

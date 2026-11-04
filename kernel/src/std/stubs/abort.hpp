#pragma once

#include <utils/panic.hpp>

namespace std {
void abort();
}

extern "C" {
void abort();
}

#pragma once

#include <cstdint>

namespace proc::timers {

// once again a whole class just to get disabled
class pit {
public:
  static void disable();
};
} // namespace proc::timers

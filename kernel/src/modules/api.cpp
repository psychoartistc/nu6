#include <modules/api.hpp>

namespace module {
extern "C" {
extern module __modules_start[];
extern module __modules_end[];
}

void initialize_some(module_init_order order) {
  for (module *current = __modules_start; current < __modules_end; ++current) {
    if (current && !current->initialized && current->order == order &&
        current->init) {
      current->init();
      current->initialized = true;
    }
  }
}
} // namespace module

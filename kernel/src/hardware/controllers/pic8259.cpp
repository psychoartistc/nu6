#include <hardware/controllers/pic8259.hpp>

namespace hardware::ctl {
static inline void io_wait() { arch::io::outb(0x80, 0); }

void pic8259::disable() {
  // master
  arch::io::outb(PIC1_DATA, 0xFF);
  io_wait();

  // slave
  arch::io::outb(PIC2_DATA, 0xFF);
  io_wait();
  s_logger.info("8259 pic disabled");
}

logging::named_logger pic8259::s_logger{"pic8259"};
} // namespace hardware::ctl

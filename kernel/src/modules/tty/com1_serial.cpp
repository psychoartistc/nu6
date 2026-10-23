#include <modules/tty/com1_serial.hpp>
#include <modules/api.hpp>
#include <vendor/printf.hpp>
#include <arch/io/ports.hpp>

namespace module {
bool com1_serial::is_transmit_empty() {
  return arch::io::inb(COM1_LINE_STATUS) & 0x20;
}

void com1_serial::put(char c) {
  while (is_transmit_empty() == 0)
    ;
  arch::io::outb(COM1_DATA, c);
}
void com1_serial::puts(std::string_view s) {
  for (const char c : s) {
    if (c == '\n')
      put('\r');

    put(c);
  }
}

void com1_serial::initialize() {

  arch::io::outb(COM1_INTERRUPT, 0x00);
  arch::io::outb(COM1_LINE_CTRL, 0x80);
  arch::io::outb(COM1_BAUD_LOW, 0x03);
  arch::io::outb(COM1_BAUD_HIGH, 0x00);
  arch::io::outb(COM1_LINE_CTRL, 0x03);
  arch::io::outb(COM1_FIFO_CTRL, 0xC7);
  arch::io::outb(COM1_MODEM_CTRL, 0x0B);

  arch::io::outb(COM1_MODEM_CTRL, 0x1E);
  arch::io::outb(COM1_DATA, 0xAE);
  if (arch::io::inb(COM1_DATA) != 0xAE) {
    m_logger.note("serial is not available");
    m_bIsError = true;
    return;
  }

  arch::io::outb(COM1_MODEM_CTRL, 0x0F);
  puts("\n\n\n\n");

  m_logger.info("initialized");
  m_bIsError = false;
}

void com1_serial::vputsf(std::string_view s, std::va_list va) {
  vfctprintf(
      [](char c, void *arg) {
        com1_serial *serial = reinterpret_cast<com1_serial *>(arg);
        if (c == '\n')
          serial->put('\r');

        serial->put(c);
      },
      this, s.data(), va);
}

void com1_serial::putsf(std::string_view s, ...) {
  std::va_list va;
  va_start(va, s);
  vputsf(s, va);
  va_end(va);
}
} // namespace module

static module::com1_serial s_serial{};

static void serial_log_handler(logging::log_level level, std::string_view name,
                               std::source_location loc, std::string_view fmt,
                               std::va_list args) {
  if (static_cast<int>(level) < static_cast<int>(logging::log_level::trace))
    return;

  std::string_view lname = logging::log_level_str(level);

  if (level != logging::log_level::printk && name.data() && name.size() >= 1) {
    s_serial.putsf("[ %s ] [ %s ] [%s:%d:%d] ", lname.data(), name.data(),
                   loc.file_name(), loc.line(), loc.column());
  } else if (level != logging::log_level::printk && (!name.data())) {
    s_serial.putsf("[ %s ] [%s:%d:%d] ", lname.data(), loc.file_name(),
                   loc.line(), loc.column());
  }

  s_serial.vputsf(fmt, args);

  if (level != logging::log_level::printk)
    s_serial.put('\n');
}

static bool com1_serial_entry() {
  s_serial.initialize();
  if (s_serial.is_error())
    return false;

  logging::log_manager::add_handler(serial_log_handler);
  return true;
}

REGISTER_MODULE(com1_serial_module, "com1 serial module",
                "polls logs to the legacy com1 serial", "1.0.0",
                module::MDLLICENSE_DEFAULT, module::module_init_order::urgent,
                com1_serial_entry, NULL);

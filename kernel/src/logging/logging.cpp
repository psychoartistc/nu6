#include <logging/logging.hpp>
#include <std/lock_guard.hpp>
#include <std/memory.hpp>

namespace logging {

void log_manager::vpoll_log(log_level level, std::string_view logger,
                            std::source_location location, const char *fmt,
                            std::va_list va) {
  std::lock_guard<std::spinlock_mutex> lock(s_lock);

  for (std::size_t i = 0; i < s_handlersCount; ++i) {
    log_handler_t &handler = s_handlers[i];
    if (!handler)
      continue;

    std::va_list newVa;
    va_copy(newVa, va);
    handler(level, logger, location, fmt, newVa);
    va_end(newVa);
  }
}

void log_manager::poll_log(log_level level, std::string_view logger,
                           std::source_location location, const char *fmt,
                           ...) {
  std::va_list va;
  va_start(va, fmt);

  vpoll_log(level, logger, location, fmt, va);

  va_end(va);
}

void log_manager::add_handler(log_handler_t h) {
  if (s_handlersCount >= MAX_HANDLERS)
    return;

  s_handlers[s_handlersCount++] = std::move(h);
}

std::size_t log_manager::s_handlersCount = 0;
std::array<log_manager::log_handler_t, log_manager::MAX_HANDLERS>
    log_manager::s_handlers = {};
std::spinlock_mutex log_manager::s_lock{};

} // namespace logging

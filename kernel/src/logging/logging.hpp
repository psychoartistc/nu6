#pragma once

#include <cstdarg>
#include <source_location>
#include <array>
#include <std/spinlock.hpp>
#include <string_view>
#include <functional>

namespace logging {
enum class log_level {
  trace,
  note,
  debug,
  info,
  warn,
  error,
  critical,
  printk
};

struct log_site {
  const char *fmt;
  std::source_location location;

  constexpr log_site(
      const char *fmt,
      std::source_location location = std::source_location::current()) noexcept
      : fmt(fmt), location(location) {}
};

static inline constexpr std::string_view log_level_str(log_level level) {
  switch (level) {
  case log_level::trace:
    return "TRACE";
  case log_level::note:
    return "NOTE";
  case log_level::debug:
    return "DEBUG";
  case log_level::info:
    return "INFO";
  case log_level::warn:
    return "WARN";
  case log_level::error:
    return "ERROR";
  case log_level::critical:
    return "CRIT";
  case log_level::printk:
    return "PRINTK";
  default:
    return "???";
  }
}

class log_manager {
public:
  using log_handler_t =
      std::function<void(log_level, std::string_view, std::source_location,
                         std::string_view, std::va_list)>;

private:
  static inline constexpr std::size_t MAX_HANDLERS = 16;
  static std::size_t s_handlersCount;
  static std::array<log_handler_t, MAX_HANDLERS> s_handlers;
  static std::spinlock_mutex s_lock;

public:
  static void vpoll_log(log_level level, std::string_view logger,
                        std::source_location location, const char *fmt,
                        std::va_list va);

  static void poll_log(log_level level, std::string_view logger,
                       std::source_location location, const char *fmt, ...);

  static void add_handler(log_handler_t h);
};

class named_logger {
public:
  constexpr explicit named_logger(std::string_view name) noexcept
      : m_name(name) {}

  constexpr std::string_view name() const noexcept { return m_name; }

  template <typename... Args> void trace(log_site site, Args &&...args) const {
    log_manager::poll_log(log_level::trace, m_name, site.location, site.fmt,
                          std::forward<Args>(args)...);
  }

  template <typename... Args> void note(log_site site, Args &&...args) const {
    log_manager::poll_log(log_level::note, m_name, site.location, site.fmt,
                          std::forward<Args>(args)...);
  }

  template <typename... Args> void debug(log_site site, Args &&...args) const {
    log_manager::poll_log(log_level::debug, m_name, site.location, site.fmt,
                          std::forward<Args>(args)...);
  }

  template <typename... Args> void info(log_site site, Args &&...args) const {
    log_manager::poll_log(log_level::info, m_name, site.location, site.fmt,
                          std::forward<Args>(args)...);
  }

  template <typename... Args> void warn(log_site site, Args &&...args) const {
    log_manager::poll_log(log_level::warn, m_name, site.location, site.fmt,
                          std::forward<Args>(args)...);
  }

  template <typename... Args> void error(log_site site, Args &&...args) const {
    log_manager::poll_log(log_level::error, m_name, site.location, site.fmt,
                          std::forward<Args>(args)...);
  }

  template <typename... Args>
  void critical(log_site site, Args &&...args) const {
    log_manager::poll_log(log_level::critical, m_name, site.location, site.fmt,
                          std::forward<Args>(args)...);
  }

  template <typename... Args> void printk(log_site site, Args &&...args) const {
    log_manager::poll_log(log_level::printk, m_name, site.location, site.fmt,
                          std::forward<Args>(args)...);
  }

  void vtrace(log_site site, std::va_list va) const {
    log_manager::vpoll_log(log_level::trace, m_name, site.location, site.fmt,
                           va);
  }

  void vnote(log_site site, std::va_list va) const {
    log_manager::vpoll_log(log_level::note, m_name, site.location, site.fmt,
                           va);
  }

  void vdebug(log_site site, std::va_list va) const {
    log_manager::vpoll_log(log_level::debug, m_name, site.location, site.fmt,
                           va);
  }

  void vinfo(log_site site, std::va_list va) const {
    log_manager::vpoll_log(log_level::info, m_name, site.location, site.fmt,
                           va);
  }

  void vwarn(log_site site, std::va_list va) const {
    log_manager::vpoll_log(log_level::warn, m_name, site.location, site.fmt,
                           va);
  }

  void verror(log_site site, std::va_list va) const {
    log_manager::vpoll_log(log_level::error, m_name, site.location, site.fmt,
                           va);
  }

  void vcritical(log_site site, std::va_list va) const {
    log_manager::vpoll_log(log_level::critical, m_name, site.location, site.fmt,
                           va);
  }

  void vprintk(log_site site, std::va_list va) const {
    log_manager::vpoll_log(log_level::printk, m_name, site.location, site.fmt,
                           va);
  }

private:
  std::string_view m_name;
};

} // namespace logging

#define pr_trace(fmt, ...)                                                     \
  logging::log_manager::poll_log(logging::log_level::trace, {},                \
                                 std::source_location::current(),              \
                                 fmt, ##__VA_ARGS__)

#define pr_note(fmt, ...)                                                      \
  logging::log_manager::poll_log(logging::log_level::note, {},                 \
                                 std::source_location::current(),              \
                                 fmt, ##__VA_ARGS__)

#define pr_debug(fmt, ...)                                                     \
  logging::log_manager::poll_log(logging::log_level::debug, {},                \
                                 std::source_location::current(),              \
                                 fmt, ##__VA_ARGS__)

#define pr_info(fmt, ...)                                                      \
  logging::log_manager::poll_log(logging::log_level::info, {},                 \
                                 std::source_location::current(),              \
                                 fmt, ##__VA_ARGS__)

#define pr_warn(fmt, ...)                                                      \
  logging::log_manager::poll_log(logging::log_level::warn, {},                 \
                                 std::source_location::current(),              \
                                 fmt, ##__VA_ARGS__)

#define pr_error(fmt, ...)                                                     \
  logging::log_manager::poll_log(logging::log_level::error, {},                \
                                 std::source_location::current(),              \
                                 fmt, ##__VA_ARGS__)

#define pr_critical(fmt, ...)                                                  \
  logging::log_manager::poll_log(logging::log_level::critical, {},             \
                                 std::source_location::current(),              \
                                 fmt, ##__VA_ARGS__)

#define printk(fmt, ...)                                                       \
  logging::log_manager::poll_log(logging::log_level::printk, {},               \
                                 std::source_location::current(),              \
                                 fmt, ##__VA_ARGS__)

#define vprintk(fmt, va)                                                       \
  logging::log_manager::vpoll_log(logging::log_level::printk, {},              \
                                  std::source_location::current(), fmt, va)

#pragma once

#include <std/spinlock.hpp>

namespace std {
template <typename T> class lock_guard {
public:
  explicit lock_guard(T &t) : m_mtx(t) { t.lock(); }

  lock_guard(const lock_guard &) = delete;
  lock_guard &operator=(const lock_guard &) = delete;

  lock_guard(lock_guard &&) = delete;
  lock_guard &operator=(lock_guard &&) = delete;

  ~lock_guard() { m_mtx.unlock(); }

private:
  T &m_mtx;
};
} // namespace std

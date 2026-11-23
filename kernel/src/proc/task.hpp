#pragma once

#include <array>
#include <arch/stack.hpp>
#include <bitset>
#include <cstdint>
#include <cstddef>

namespace proc {
class cfs;
class run_queue;
class task {
  friend class cfs;
  friend class run_queue;

public:
  typedef void (*entry_function_t)(void *arg);

  enum class flags {
    ready,
    running,
    blocked,
    dead,
    sleeping,
    idle,
    user,

    block_req
  };

  static inline constexpr std::array<flags, 5> STATES = {
      flags::blocked, flags::dead, flags::ready, flags::sleeping,
      flags::running};

  static inline constexpr int FLAGS_COUNT = static_cast<int>(flags::block_req);

private:
  arch::stack m_stack;
  std::uint64_t m_id{0};
  std::uintptr_t m_rsp{0};
  std::uint64_t m_wakeNs{0};

  task *m_sleepNext{nullptr};

  std::int32_t m_nice{0};
  std::uint32_t m_weight{0};

  std::uint32_t m_cpu{0};
  std::uint64_t m_vruntime{0};
  std::uint64_t m_sumExec{0};
  std::uint64_t m_execStart{0};
  std::uint64_t m_sliceStart{0};

  entry_function_t m_entryFunction{nullptr};
  void *m_arg{nullptr};

  std::bitset<FLAGS_COUNT> m_flags{0};
  bool m_bQueued{false};

private:
public:
  inline flags get_state() const {
    for (const enum flags f : STATES)
      if (has_flag(f))
        return f;

    return flags::dead;
  }

  std::uint64_t vruntime() const { return m_vruntime; }

  const std::bitset<FLAGS_COUNT> &flags() const { return m_flags; }

  bool has_flag(enum flags fl) const {
    return m_flags.test(static_cast<int>(fl));
  }
};
} // namespace proc

extern "C" {
extern void task_switch(std::uint64_t *old, std::uint64_t newRsp);
extern void task_trampoline(void);
void task_first_run(void);
void task_exit(void);
}

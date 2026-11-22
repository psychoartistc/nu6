#pragma once

#include <arch/stack.hpp>
#include <bitset>
#include <cstdint>
#include <cstddef>

namespace proc {
class task {
public:
  typedef void (*entry_function_t)(void *arg);

  enum class flags {
    ready,
    running,
    blocked,
    dead,
    sleeping,
    user,
    idle,
    block_req
  };

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
  void *arg{nullptr};

  std::bitset<FLAGS_COUNT> m_flags;

private:
};
} // namespace proc

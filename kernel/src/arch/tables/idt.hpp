#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <logging/logging.hpp>

namespace arch {

struct [[gnu::packed]] interrupt_frame {
  uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
  uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax, int_no, err_code;
  uint64_t rip, cs, rflags, rsp, ss;
};

class idt {
public:
  static constexpr std::size_t ENTRY_COUNT = 256;

  using handler_t = void (*)();

  struct [[gnu::packed]] entry {
    std::uint16_t offset_low;
    std::uint16_t selector;

    std::uint8_t ist;

    std::uint8_t type_attr;

    std::uint16_t offset_middle;
    std::uint32_t offset_high;

    std::uint32_t reserved;
  };

  struct [[gnu::packed]] ptr {
    std::uint16_t limit;
    std::uint64_t base;
  };

private:
  static std::array<entry, ENTRY_COUNT> s_idt;
  static ptr s_idtPtr;
  static logging::named_logger s_logger;

public:
  static bool set_gate(std::uint8_t vector, handler_t handler,
                       std::uint16_t selector = 0x08, std::uint8_t ist = 0,
                       std::uint8_t type_attr = 0x8E);

  static void clear_gate(std::uint8_t vector);

  static void initialize_and_load();

  static void reload();
};

} // namespace arch

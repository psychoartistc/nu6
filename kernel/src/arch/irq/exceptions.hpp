#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace arch {
namespace irq {

class exceptions {
  static inline constexpr std::uint64_t EXCEPTION_NUM = 32;

  static inline constexpr std::array<std::string_view, EXCEPTION_NUM>
      EXCEPTION_NAMES = {"Divide error",
                         "Debug",
                         "NMI interrupt",
                         "Breakpoint",
                         "Overflow",
                         "BOUND range exceeded",
                         "Invalid opcode",
                         "Device not available",
                         "Double fault",
                         "Coprocessor segment overrun",
                         "Invalid TSS",
                         "Segment not present",
                         "Stack segment fault",
                         "General protection",
                         "Page fault",
                         "Reserved",
                         "x87 FPU floating point rror",
                         "Alignment check",
                         "Machine check",
                         "SIMD floating point exception",
                         "Virtualization exception",
                         "Control protection exception",
                         "Reserved",
                         "Reserved",
                         "Reserved",
                         "Reserved",
                         "Reserved",
                         "Reserved",
                         "Hypervisor injection exception",
                         "VMM communication exception",
                         "Security exception",
                         "Reserved"};

  static inline constexpr std::uint64_t CPU_EXCEPT_PAGE_FAULT = 14;
  static inline constexpr std::uint64_t CPU_EXCEPT_DOUBLE_FAULT = 8;
  static inline constexpr std::uint64_t CPU_EXCEPT_NMI = 2;
  static inline constexpr std::uint64_t CPU_EXCEPT_DEBUG = 1;
  static inline constexpr std::uint64_t CPU_EXCEPT_MACHINE_CHECK = 18;

  static inline constexpr std::uint64_t IST_EXCEPTIONS_NUM = 5;

  static inline constexpr std::array<std::uint64_t, IST_EXCEPTIONS_NUM>
      IST_EXCEPTIONS_INT_NO = {CPU_EXCEPT_PAGE_FAULT, CPU_EXCEPT_DOUBLE_FAULT,
                               CPU_EXCEPT_NMI, CPU_EXCEPT_DEBUG,
                               CPU_EXCEPT_MACHINE_CHECK};

public:
  static void install();
};
} // namespace irq
} // namespace arch

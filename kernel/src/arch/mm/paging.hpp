#pragma once

#include <cstdint>
#include <cstddef>

namespace arch {
namespace mm {

static inline constexpr std::uint64_t MASK_4K = 0x000FFFFFFFFFF000ULL;
static inline constexpr std::uint64_t MASK_2M = 0x000FFFFFFFE00000ULL;
static inline constexpr std::uint64_t MASK_1G = 0x000FFFFFC0000000ULL;

static inline constexpr std::uint64_t P_X86_PRESENT(1ULL << 0);
static inline constexpr std::uint64_t P_X86_WRITABLE(1ULL << 1);
static inline constexpr std::uint64_t P_X86_USER(1ULL << 2);
static inline constexpr std::uint64_t P_X86_WRITETHRU(1ULL << 3);
static inline constexpr std::uint64_t P_X86_NOCACHE(1ULL << 4);
static inline constexpr std::uint64_t P_X86_HUGE(1ULL << 7);
static inline constexpr std::uint64_t P_X86_GLOBAL(1ULL << 8);
static inline constexpr std::uint64_t P_X86_NX(1ULL << 63);
static inline constexpr std::uint64_t P_X86_OWNED(1ULL << 9);

static inline constexpr std::uint64_t PT_FLAGS_MASK =
    (P_X86_PRESENT | P_X86_WRITABLE | P_X86_USER | P_X86_WRITETHRU |
     P_X86_NOCACHE | P_X86_NX);

static inline constexpr std::uint64_t PTE_ACCESSED(1ULL << 5);
static inline constexpr std::uint64_t PTE_DIRTY(1ULL << 6);
static inline constexpr std::uint64_t PTE_PAT_HUGE(1ULL << 12);

#define PML4_INDEX(a) (((a) >> 39) & 0x1FF)
#define PDPT_INDEX(a) (((a) >> 30) & 0x1FF)
#define PD_INDEX(a) (((a) >> 21) & 0x1FF)
#define PT_INDEX(a) (((a) >> 12) & 0x1FF)

static inline uint64_t read_cr3(void) {
  uint64_t v;
  asm volatile("mov %%cr3, %0" : "=r"(v));
  return v;
}

static inline void write_cr3(uint64_t cr3) {
  asm volatile("movq %0, %%cr3" : : "r"(cr3) : "memory");
}

static inline std::uint64_t read_cr2(void) {
  std::uint64_t v;
  asm volatile("mov %%cr2, %0" : "=r"(v));
  return v;
}
} // namespace mm
} // namespace arch

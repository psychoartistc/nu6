#pragma once

#include <limine/requests.hpp>
#include <cstddef>
#include <cstdint>

namespace mm {
// 4kb
static inline constexpr std::size_t PAGE_SIZE = 4096;

// 2mb
static inline constexpr std::size_t SUPERPAGE_SIZE = 2097152;

// 1gb
static inline constexpr std::size_t MEGAPAGE_SIZE = 1073741824;

static inline void *p2v(std::uintptr_t phys) {
  return reinterpret_cast<void *>(phys + limine::hhdm_request.response->offset);
}

static inline std::uintptr_t v2p(void *virt) {
  return reinterpret_cast<std::uintptr_t>(virt) -
         limine::hhdm_request.response->offset;
}
} // namespace mm

#include <std/memory.hpp>
#include <utils/math.hpp>
#include <limine.h>
#include <std/enums.hpp>
#include <mm/physical/page_allocator.hpp>
#include <arch/tables/gdt.hpp>
#include <logging/logging.hpp>
#include <modules/api.hpp>
#include <stdbool.h>
#include <stdint.h>
#include <stdnoreturn.h>
#include <std/spinlock.hpp>
#include <abi/itanium.hpp>
#include <arch/tables/tss.hpp>
#include <arch/tables/idt.hpp>
#include <arch/tables/ist.hpp>
#include <std/dsa/rbtree.hpp>
#include <mm/physical/freelist/freelist.hpp>
template <typename T> class bump_allocator {
private:
  std::uint8_t m_chunk[mm::PAGE_SIZE * 2]{};
  std::uint64_t m_offset{0x0};

public:
  using value_type = T;

  bump_allocator() noexcept = default;

  template <typename U> bump_allocator(const bump_allocator<U> &) noexcept {}

  T *allocate(std::size_t n) {
    if ((int64_t)mm::PAGE_SIZE - (int64_t)n * sizeof(T) < m_offset) {
      return nullptr;
    }

    std::uint8_t *chunk = m_chunk + m_offset;
    m_offset += n * sizeof(T);

    return reinterpret_cast<T *>(chunk);
  }

  void deallocate(T *p, std::size_t n) noexcept {
    (void)p;
    (void)n;
  }

  template <typename U>
  bool operator==(const bump_allocator<U> &) const noexcept {
    return true;
  }

  template <typename U>
  bool operator!=(const bump_allocator<U> &) const noexcept {
    return false;
  }
};

extern "C" void kmain() {
  cxxabi::call_global_ctors();
  module::initialize_some(module::module_init_order::urgent);

  mm::page_allocator::initialize();

  auto res = mm::page_allocator::allocate(1);
  if (!res)
    pr_error("err: %d", static_cast<int>(res.error()));
  else
    pr_info("0x%p", *res);

  std::rb_tree<int, std::rb_identity, std::less<>, bump_allocator<int>> rb;
  rb.insert(123);
  rb.insert(5050);
  for (int i : rb) {
    pr_info("%d", i);
  }

  arch::hcf();
}

#include <hardware/controllers/lapic.hpp>
#include <vendor/magic_enum.hpp>
#include <mm/kernel_mm.hpp>
#include <mm/virtual/vma_tree.hpp>
#include <arch/io/msr.hpp>

namespace hardware::ctl {
void lapic::initialize(std::uintptr_t base) {
  s_physBase = base;
  s_isX2Apic = (arch::io::rdmsr(MSR_IA32_APIC_BASE) & APIC_BASE_X2APIC_EN) != 0;

  auto &vma_tree = mm::get_kernel_vma_tree();

  mm::vma *vma = vma_tree.allocate(mm::PAGE_SIZE);
  assert(vma && "failed to allocate vma for lapic, fatal");

  auto &pt = mm::get_kernel_pt();
  auto res = pt.map(base, vma->address,
                    mm::protection::kernel | mm::protection::nocache |
                        mm::protection::write | mm::protection::read);
  if (!res)
    utils::panic("failed to map lapic to virtual memory, fatal: %s",
                 magic_enum::enum_name(res.error()));

  s_logger.info("initialized at virtual 0x%p, x2apic enabled: %s", vma->address,
                s_isX2Apic ? "yes" : "no");

  s_mmio.change_address(vma->address);
}

std::uintptr_t lapic::s_physBase{0x0};
utils::mmio lapic::s_mmio{0x0};
bool lapic::s_isX2Apic{false};
logging::named_logger lapic::s_logger{"lapic"};
} // namespace hardware::ctl

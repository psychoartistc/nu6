#include <hardware/controllers/lapic.hpp>

namespace hardware::ctl {
lapic::lapic(std::uintptr_t addr) : m_physBase(addr) {}
} // namespace hardware::ctl

#include <memory>
#include <proc/process.hpp>

namespace proc {
void process::on_page_fault(std::uintptr_t fault) {
  pr_info("0x%p faulted in userland!", fault);
}

void process::set_current() { s_pCurrent = shared_from_this(); }

mm::address_space &process::address_space() { return m_addressSpace; }

std::shared_ptr<process> process::s_pCurrent{nullptr};

std::shared_ptr<process> process::current() { return s_pCurrent; }
void process::enter_kernel() { s_pCurrent = nullptr; }
} // namespace proc

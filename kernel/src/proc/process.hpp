#pragma once

#include <optional>
#include <mm/address_space.hpp>
#include <memory>

namespace proc {
class process : public std::enable_shared_from_this<process> {
private:
  // if this is null that means we are in kernel space
  static std::shared_ptr<process> s_pCurrent;

private:
  mm::address_space m_addressSpace;

public:
  void set_current();

  void on_page_fault(std::uintptr_t fault);

  mm::address_space &address_space();

  static std::shared_ptr<process> current();
  static void enter_kernel();
};
} // namespace proc

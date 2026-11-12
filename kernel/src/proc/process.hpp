#pragma once

#include <optional>
#include <std/vendor/veque.hpp>
#include <mm/address_space.hpp>
#include <memory>

namespace proc {
class process : public std::enable_shared_from_this<process> {
public:
  using pointer = std::shared_ptr<process>;

private:
  // if this is null that means we are in kernel space
private:
  mm::address_space m_addressSpace;
  std::vector<pointer> m_children;

  static pointer s_pCurrent;

public:
  void set_current();

  void on_exception(std::uint64_t int_no);

  mm::address_space &address_space();

  static pointer current();
  static void enter_kernel();
};
} // namespace proc

#include <mm/virtual/tlb.hpp>
#include <logging/logging.hpp>

namespace mm {
namespace tlb {
void on_shootdown(page_table &tbl, const flush_batch &batch) {
  // pr_info("shootdown");
}
} // namespace tlb
} // namespace mm

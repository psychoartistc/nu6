#pragma once

#include <limine.h>

namespace limine {

extern volatile uint64_t limine_base_revision[];
extern volatile uint64_t limine_requests_start_marker[];
extern volatile struct limine_framebuffer_request framebuffer_request;
extern volatile struct limine_memmap_request memmap_request;
extern volatile struct limine_hhdm_request hhdm_request;
extern volatile struct limine_executable_address_request kernel_address_request;
extern volatile struct limine_executable_file_request kernel_file_request;
extern volatile struct limine_rsdp_request rsdp_request;
extern volatile struct limine_mp_request mp_request;
extern volatile struct limine_stack_size_request stacksize_request;
extern volatile struct limine_dtb_request dtb_request;
extern volatile struct limine_module_request module_request;
extern volatile uint64_t limin_requests_end_marker[];

static inline bool framebuffer_available() {
  return framebuffer_request.response &&
         framebuffer_request.response->framebuffer_count &&
         framebuffer_request.response->framebuffer_count;
}
} // namespace limine

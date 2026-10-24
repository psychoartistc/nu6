#include <limine/requests.hpp>

#define __LIMINE_REQUEST [[gnu::used, gnu::section(".limine_requests")]]

namespace limine {
__LIMINE_REQUEST volatile uint64_t limine_base_revision[] =
    LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests_start"))) volatile uint64_t
    limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__LIMINE_REQUEST volatile struct limine_framebuffer_request
    framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
                           .revision = 0,
                           .response = nullptr};

__LIMINE_REQUEST volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
    .response = nullptr,
};

__LIMINE_REQUEST volatile struct limine_executable_file_request
    kernel_file_request = {.id = LIMINE_EXECUTABLE_FILE_REQUEST_ID,
                           .revision = 0,
                           .response = nullptr};

__LIMINE_REQUEST volatile struct limine_dtb_request dtb_request = {
    .id = LIMINE_DTB_REQUEST_ID, .revision = 0, .response = nullptr};

__LIMINE_REQUEST volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID, .revision = 0, .response = nullptr};

__LIMINE_REQUEST volatile struct limine_executable_address_request
    kernel_address_request = {.id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
                              .revision = 0,
                              .response = nullptr};
__LIMINE_REQUEST volatile struct limine_mp_request mp_request = {
    .id = LIMINE_MP_REQUEST_ID, .revision = 0, .response = nullptr, .flags = 0};

__LIMINE_REQUEST volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST_ID, .revision = 0, .response = nullptr};

__LIMINE_REQUEST volatile struct limine_stack_size_request stacksize_request = {
    .id = LIMINE_STACK_SIZE_REQUEST_ID,
    .revision = 0,
    .response = nullptr,
    .stack_size = 1024 * 1024 * 2};

__LIMINE_REQUEST volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 0,
    .response = nullptr,
    .internal_module_count = 0,
    .internal_modules = 0x0};

[[gnu::used,
  gnu::section(
      ".limine_requests_end")]] volatile uint64_t limine_requests_end_marker[] =
    LIMINE_REQUESTS_END_MARKER;
} // namespace limine

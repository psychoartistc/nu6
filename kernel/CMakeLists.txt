include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.deps-obtained)
    execute_process(
        COMMAND ./get-deps
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE GET_DEPS_RESULT
    )
    if(NOT GET_DEPS_RESULT EQUAL 0)
        message(FATAL_ERROR "./get-deps failed")
    endif()
endif()

set(KERNEL_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE KERNEL_SOURCES
    FOLLOW_SYMLINKS
    CONFIGURE_DEPENDS
    ${KERNEL_DIR}/src/*.c
    ${KERNEL_DIR}/src/*.cpp
    ${KERNEL_DIR}/src/*.S
    ${KERNEL_DIR}/src/*.asm
    ${KERNEL_DIR}/cc-runtime/src/*.c
    ${KERNEL_DIR}/cc-runtime/src/*.cpp
    ${KERNEL_DIR}/cc-runtime/src/*.S
    ${KERNEL_DIR}/cc-runtime/src/*.asm
)
list(SORT KERNEL_SOURCES)

add_executable(kernel ${KERNEL_SOURCES})

set_target_properties(kernel PROPERTIES
    OUTPUT_NAME kernel
    SUFFIX ""
    LINKER_LANGUAGE CXX
    LINK_DEPENDS ${KERNEL_DIR}/link.lds
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin-x86_64
    POSITION_INDEPENDENT_CODE OFF
)

set(CMAKE_ASM_NASM_FLAGS "-g -F dwarf -i ${KERNEL_DIR}/src/ -Wall")

set(TO_STRIP "${CMAKE_SOURCE_DIR}")

string(REGEX MATCH "^([^/]+/[^/]+/[^/]+)" VISUAL_ROOT "${TO_STRIP}")

set(TARGET_STRIP_DIR "${CMAKE_SOURCE_DIR}/kernel/src")


set(KERNEL_COMMON_OPTIONS
    -g
    -O2
    -pipe
    -Wall
    -Wextra
    -ffreestanding
    -fno-common
    -fno-stack-protector
    -fno-stack-check
    -fno-PIC
    -ffunction-sections
    -fdata-sections
    -m64
    -march=x86-64
    -mno-mmx
    -mno-sse
    -mno-red-zone
    -mcmodel=kernel
    -nostdinc
    -fmacro-prefix-map=${TARGET_STRIP_DIR}/=
)

check_c_compiler_flag(-fno-stack-clash-protection HAS_C_NO_STACK_CLASH)
check_c_compiler_flag(-fcf-protection=none HAS_C_CF_NONE)
check_c_compiler_flag(-mno-80387 HAS_C_NO_80387)
check_cxx_compiler_flag(-fno-stack-clash-protection HAS_CXX_NO_STACK_CLASH)
check_cxx_compiler_flag(-fcf-protection=none HAS_CXX_CF_NONE)
check_cxx_compiler_flag(-mno-80387 HAS_CXX_NO_80387)

set(KERNEL_C_OPTIONS -std=gnu11)
#if(HAS_C_NO_STACK_CLASH)
#     list(APPEND KERNEL_C_OPTIONS -fno-stack-clash-protection)
#endif()
if(HAS_C_CF_NONE)
    list(APPEND KERNEL_C_OPTIONS -fcf-protection=none)
endif()
if(HAS_C_NO_80387)
    list(APPEND KERNEL_C_OPTIONS -mno-80387)
endif()

set(KERNEL_CXX_OPTIONS -std=c++23 -fno-rtti -fno-exceptions)
#if(HAS_CXX_NO_STACK_CLASH)
  # list(APPEND KERNEL_CXX_OPTIONS -fno-stack-clash-protection)
#endif()
if(HAS_CXX_CF_NONE)
    list(APPEND KERNEL_CXX_OPTIONS -fcf-protection=none)
endif()
if(HAS_CXX_NO_80387)
    list(APPEND KERNEL_CXX_OPTIONS -mno-80387)
endif()

target_compile_options(kernel PRIVATE
    "$<$<COMPILE_LANGUAGE:C,CXX,ASM>:${KERNEL_COMMON_OPTIONS}>"
    "$<$<COMPILE_LANGUAGE:C,ASM>:${KERNEL_C_OPTIONS}>"
    "$<$<COMPILE_LANGUAGE:CXX>:${KERNEL_CXX_OPTIONS}>"
)

target_compile_definitions(kernel PRIVATE
    PAGE_ALLOC_USE_FREELIST
    "__noreturn__="
)

target_include_directories(kernel PRIVATE
    ${KERNEL_DIR}/src
    ${KERNEL_DIR}/limine-protocol/include
)

target_include_directories(kernel SYSTEM PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${KERNEL_DIR}/freestanding-cxx-hdrs/include>"
    ${KERNEL_DIR}/freestanding-c-hdrs/x86_64/include
)

if(EXISTS ${KERNEL_DIR}/src/lib/memory.cpp)
    set_source_files_properties(${KERNEL_DIR}/src/lib/memory.cpp
        PROPERTIES COMPILE_OPTIONS -fno-lto)
endif()
if(EXISTS ${KERNEL_DIR}/cc-runtime/src/cc-runtime.c)
    set_source_files_properties(${KERNEL_DIR}/cc-runtime/src/cc-runtime.c
        PROPERTIES COMPILE_OPTIONS -fno-lto)
endif()

target_link_options(kernel PRIVATE
    -fuse-ld=lld
    -Wl,--build-id=sha1
    -nostdlib
    -static
    -no-pie
    -Wl,-z,max-page-size=0x1000
    -Wl,-z,noexecstack
    -Wl,--gc-sections
    -Wl,-T,${KERNEL_DIR}/link.lds
    -Wl,-m,elf_x86_64
)

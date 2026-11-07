cmake_minimum_required(VERSION 3.20)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_ASM_COMPILER clang)
set(CMAKE_ASM_NASM_COMPILER nasm)
set(CMAKE_C_COMPILER_TARGET x86_64-unknown-none-elf)
set(CMAKE_CXX_COMPILER_TARGET x86_64-unknown-none-elf)
set(CMAKE_ASM_COMPILER_TARGET x86_64-unknown-none-elf)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

project(nu6 LANGUAGES C CXX ASM ASM_NASM)

set(IMAGE_NAME nu6)

set(HOST_CC gcc)
set(HOST_CFLAGS "-g -O2 -pipe")
set(HOST_CPPFLAGS "")
set(HOST_LDFLAGS "")
set(HOST_LIBS "")

set(LIMINE_EFI BOOTX64.EFI BOOTIA32.EFI)
set(XORRISO_BIOS_FLAGS
    -b boot/limine/limine-bios-cd.bin
    -no-emul-boot
    -boot-load-size 4
    -boot-info-table
)

set(LIMINE_DIR ${CMAKE_BINARY_DIR}/limine-binary)
set(ISO_ROOT ${CMAKE_BINARY_DIR}/iso_root)
set(ISO_FILE ${CMAKE_BINARY_DIR}/${IMAGE_NAME}.iso)

set(LIMINE_EFI_FILES ${LIMINE_EFI})
list(TRANSFORM LIMINE_EFI_FILES PREPEND ${LIMINE_DIR}/)

add_subdirectory(kernel)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/limine-binary.tar.gz
    COMMAND curl -fL -o ${CMAKE_BINARY_DIR}/limine-binary.tar.gz
            https://github.com/Limine-Bootloader/Limine/releases/latest/download/limine-binary.tar.gz
    VERBATIM
)

add_custom_command(
    OUTPUT ${LIMINE_DIR}/limine
    DEPENDS ${CMAKE_BINARY_DIR}/limine-binary.tar.gz
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${LIMINE_DIR}
    COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_BINARY_DIR}/limine-binary.tar.gz
    COMMAND make -C ${LIMINE_DIR}
            "CC=${HOST_CC}"
            "CFLAGS=${HOST_CFLAGS}"
            "CPPFLAGS=${HOST_CPPFLAGS}"
            "LDFLAGS=${HOST_LDFLAGS}"
            "LIBS=${HOST_LIBS}"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
)

add_custom_command(
    OUTPUT ${ISO_FILE}
    DEPENDS
        ${LIMINE_DIR}/limine
        kernel
        ${CMAKE_SOURCE_DIR}/limine.conf
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${ISO_ROOT}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ISO_ROOT}/boot/limine
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel> ${ISO_ROOT}/boot/
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/limine.conf
            ${LIMINE_DIR}/limine-uefi-cd.bin
            ${LIMINE_DIR}/limine-bios.sys
            ${LIMINE_DIR}/limine-bios-cd.bin
            ${ISO_ROOT}/boot/limine/
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${ISO_ROOT}/assets
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ISO_ROOT}/EFI/BOOT
    COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_EFI_FILES} ${ISO_ROOT}/EFI/BOOT/
    COMMAND xorriso -as mkisofs -R -r -J ${XORRISO_BIOS_FLAGS}
            -hfsplus -apm-block-size 2048
            --efi-boot boot/limine/limine-uefi-cd.bin
            -efi-boot-part --efi-boot-image --protective-msdos-label
            ${ISO_ROOT} -o ${ISO_FILE}
    COMMAND ${LIMINE_DIR}/limine bios-install ${ISO_FILE}
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${ISO_ROOT}
    VERBATIM
)

add_custom_target(iso ALL DEPENDS ${ISO_FILE})

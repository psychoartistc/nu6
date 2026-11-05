#!/bin/bash
set -e

if [ ! -d edk2-ovmf-bins ]; then
    curl -fL -o edk2-ovmf-bins.tar.gz https://github.com/osdev0/edk2-ovmf-stable-bins/releases/latest/download/edk2-ovmf-bins.tar.gz
    gunzip < edk2-ovmf-bins.tar.gz | tar -xf -
    rm -f edk2-ovmf-bins.tar.gz
fi

cmake -S . -B build -G Ninja && cmake --build build && qemu-system-x86_64 \
    -drive if=pflash,unit=0,format=raw,file=edk2-ovmf-bins/ovmf-code-x86_64.fd,readonly=on \
    -M q35 \
    -cdrom build/nu6.iso \
    -m 2G -smp 4 -serial stdio -enable-kvm -cpu host,+tsc-deadline,+pdpe1gb,+invtsc,+nx

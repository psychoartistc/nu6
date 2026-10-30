.SUFFIXES:

.DELETE_ON_ERROR:

QEMUFLAGS := -m 2G -serial stdio -enable-kvm -cpu host,+tsc-deadline,+pdpe1gb,+invtsc,+nx

override XORRISO_BIOS_FLAGS :=
override SGDISK_MBR_FLAGS :=

    override QEMU_MACHINE_FLAGS := \
        -M q35
    override LIMINE_EFI := BOOTX64.EFI BOOTIA32.EFI
    override XORRISO_BIOS_FLAGS := \
        -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table
    override SGDISK_MBR_FLAGS := -m 1

override QEMU_UEFI_FLAGS := \
    -drive if=pflash,unit=0,format=raw,file=edk2-ovmf-bins/ovmf-code-x86_64.fd,readonly=on

override IMAGE_NAME := template-x86_64

HDD_SIZE := 64

override HDD_HEADS := 64
override HDD_SECTORS_PER_TRACK := 32
override HDD_CYLINDER_SECTORS := $(shell echo $$(( $(HDD_HEADS) * $(HDD_SECTORS_PER_TRACK) )))

override HDD_PART_START := $(HDD_CYLINDER_SECTORS)
override HDD_PART_SECTORS := $(shell echo $$(( ($(HDD_SIZE) - 2) * $(HDD_CYLINDER_SECTORS) )))
override HDD_PART_END := $(shell echo $$(( $(HDD_PART_START) + $(HDD_PART_SECTORS) - 1 )))
override HDD_PART_OFFSET := $(shell echo $$(( $(HDD_PART_START) * 512 )))

HOST_CC := gcc
HOST_CFLAGS := -g -O2 -pipe
HOST_CPPFLAGS :=
HOST_LDFLAGS :=
HOST_LIBS :=

.PHONY: all
all: $(IMAGE_NAME).iso

.PHONY: all-hdd
all-hdd: $(IMAGE_NAME).hdd

.PHONY: run
run: edk2-ovmf-bins $(IMAGE_NAME).iso
	qemu-system-x86_64 \
		$(QEMU_MACHINE_FLAGS) \
		$(QEMU_UEFI_FLAGS) \
		-cdrom $(IMAGE_NAME).iso \
		$(QEMUFLAGS)

.PHONY: run-hdd
run-hdd: edk2-ovmf-bins $(IMAGE_NAME).hdd
	qemu-system-x86_64 \
		$(QEMU_MACHINE_FLAGS) \
		$(QEMU_UEFI_FLAGS) \
		-hda $(IMAGE_NAME).hdd \
		$(QEMUFLAGS)

.PHONY: run-bios
run-bios: $(IMAGE_NAME).iso
	qemu-system-x86_64 \
		$(QEMU_MACHINE_FLAGS) \
		-cdrom $(IMAGE_NAME).iso \
		-boot d \
		$(QEMUFLAGS)

.PHONY: run-hdd-bios
run-hdd-bios: $(IMAGE_NAME).hdd
	qemu-system-x86_64 \
		$(QEMU_MACHINE_FLAGS) \
		-hda $(IMAGE_NAME).hdd \
		$(QEMUFLAGS)


.INTERMEDIATE: edk2-ovmf-bins.tar.gz
edk2-ovmf-bins.tar.gz:
	curl -fL -o $@ https://github.com/osdev0/edk2-ovmf-stable-bins/releases/latest/download/edk2-ovmf-bins.tar.gz

edk2-ovmf-bins: edk2-ovmf-bins.tar.gz
	rm -rf edk2-ovmf-bins
	gunzip < edk2-ovmf-bins.tar.gz | tar -xf -

.INTERMEDIATE: limine-binary.tar.gz
limine-binary.tar.gz:
	curl -fL -o $@ https://github.com/Limine-Bootloader/Limine/releases/latest/download/limine-binary.tar.gz

limine-binary/limine: limine-binary.tar.gz
	rm -rf limine-binary
	gunzip < limine-binary.tar.gz | tar -xf -
	$(MAKE) -C limine-binary \
		CC="$(HOST_CC)" \
		CFLAGS="$(HOST_CFLAGS)" \
		CPPFLAGS="$(HOST_CPPFLAGS)" \
		LDFLAGS="$(HOST_LDFLAGS)" \
		LIBS="$(HOST_LIBS)"

kernel/.deps-obtained:
	./kernel/get-deps

.PHONY: kernel
kernel: kernel/.deps-obtained
	$(MAKE) -C kernel

$(IMAGE_NAME).iso: limine-binary/limine kernel
	rm -rf iso_root
	mkdir -p iso_root/boot
	cp -v kernel/bin-x86_64/kernel iso_root/boot/
	mkdir -p iso_root/boot/limine
	cp -v limine.conf limine-binary/limine-uefi-cd.bin iso_root/boot/limine/
	cp -v -r assets iso_root/assets
	cp -v limine-binary/limine-bios.sys limine-binary/limine-bios-cd.bin iso_root/boot/limine/

	mkdir -p iso_root/EFI/BOOT
	cp -v $(addprefix limine-binary/,$(LIMINE_EFI)) iso_root/EFI/BOOT/
	xorriso -as mkisofs -R -r -J $(XORRISO_BIOS_FLAGS) \
		-hfsplus -apm-block-size 2048 \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(IMAGE_NAME).iso

	./limine-binary/limine bios-install $(IMAGE_NAME).iso

	rm -rf iso_root

$(IMAGE_NAME).hdd: limine-binary/limine kernel
	rm -f $(IMAGE_NAME).hdd
	dd if=/dev/zero bs=1024k count=0 seek=$(HDD_SIZE) of=$(IMAGE_NAME).hdd
	PATH=$$PATH:/usr/sbin:/sbin sgdisk $(IMAGE_NAME).hdd -n 1:$(HDD_PART_START):$(HDD_PART_END) -t 1:ef00 $(SGDISK_MBR_FLAGS)

	./limine-binary/limine bios-install $(IMAGE_NAME).hdd

	mformat -i $(IMAGE_NAME).hdd@@$(HDD_PART_OFFSET) -T $(HDD_PART_SECTORS) -h $(HDD_HEADS) -s $(HDD_SECTORS_PER_TRACK) ::
	mmd -i $(IMAGE_NAME).hdd@@$(HDD_PART_OFFSET) ::/EFI ::/EFI/BOOT ::/boot ::/boot/limine
	mcopy -i $(IMAGE_NAME).hdd@@$(HDD_PART_OFFSET) kernel/bin-x86_64/kernel ::/boot
	mcopy -i $(IMAGE_NAME).hdd@@$(HDD_PART_OFFSET) limine.conf ::/boot/limine

	mcopy -i $(IMAGE_NAME).hdd@@$(HDD_PART_OFFSET) limine-binary/limine-bios.sys ::/boot/limine

	mcopy -i $(IMAGE_NAME).hdd@@$(HDD_PART_OFFSET) $(addprefix limine-binary/,$(LIMINE_EFI)) ::/EFI/BOOT

.PHONY: clean
clean:
	$(MAKE) -C kernel clean
	rm -rf iso_root $(IMAGE_NAME).iso $(IMAGE_NAME).hdd

.PHONY: distclean
distclean:
	$(MAKE) -C kernel distclean
	rm -rf iso_root *.iso *.hdd limine-binary limine-binary.tar.gz edk2-ovmf-bins edk2-ovmf-bins.tar.gz


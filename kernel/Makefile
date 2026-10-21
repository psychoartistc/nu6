.SUFFIXES:

.DELETE_ON_ERROR:

override OUTPUT := kernel


PREFIX := /usr/local

CFLAGS := -g -O2 -pipe -DPAGE_ALLOC_USE_LLFREE

TOOLCHAIN := 
TOOLCHAIN_PREFIX := x86_64-elf-
ifneq ($(TOOLCHAIN),)
    ifeq ($(TOOLCHAIN_PREFIX),)
        TOOLCHAIN_PREFIX := $(TOOLCHAIN)-
    endif
endif

ifneq ($(TOOLCHAIN_PREFIX),)
    CC := $(TOOLCHAIN_PREFIX)gcc
    CXX := $(TOOLCHAIN_PREFIX)g++
else
    CC := cc
    CXX := c++
endif

ifeq ($(TOOLCHAIN),llvm)
    CC := clang
    CXX := clang++
		#override CFLAGS += -D_Atomic=
endif

CXXFLAGS := $(CFLAGS)

override CFLAGS := $(CFLAGS)
override CXXFLAGS := $(CXXFLAGS)

CPPFLAGS :=

NASMFLAGS := -g

LDFLAGS :=

ifneq ($(filter-out clean distclean,$(or $(MAKECMDGOALS),default)),)
    ifeq ($(wildcard .deps-obtained),)
        $(error Please run the ./get-deps script first)
    endif
endif

ifneq ($(filter-out clean distclean,$(or $(MAKECMDGOALS),default)),)
    override TOOLCHAIN_ERROR = $(error $(1))
else
    override TOOLCHAIN_ERROR =
endif

override CC_IS_CLANG := $(shell ! $(CC) --version 2>/dev/null | grep -q '^Target: '; echo $$?)

override CXX_IS_CLANG := $(shell ! $(CXX) --version 2>/dev/null | grep -q '^Target: '; echo $$?)

override CFLAGS_HAS_LTO := $(shell ! printf '%s\n' $(CFLAGS) | grep -E '^-f(no-)?lto(=|$$)' | tail -n 1 | grep -q '^-flto'; echo $$?)

override CXXFLAGS_HAS_LTO := $(shell ! printf '%s\n' $(CXXFLAGS) | grep -E '^-f(no-)?lto(=|$$)' | tail -n 1 | grep -q '^-flto'; echo $$?)

override FLAG_NAME = -$(subst -,-?,$(patsubst -%,%,$(firstword $(subst =, ,$(1)))))

override FLAG_IS_NAMED = grep -qE '(error|warning|note):.*[^[:alnum:]]($(call FLAG_NAME,$(1))$(if $(2),|$(call FLAG_NAME,$(2))))([^-[:alnum:]]|$$)'
override CC_HAS_COMPILE_FLAG = $(shell LC_ALL=C $(CC) $(CFLAGS) $(1) -c -x c /dev/null -o /dev/null 2>&1 >/dev/null | $(call FLAG_IS_NAMED,$(1),$(2)); echo $$?)

override CXX_HAS_COMPILE_FLAG = $(shell LC_ALL=C $(CXX) $(CXXFLAGS) $(1) -c -x c++ /dev/null -o /dev/null 2>&1 >/dev/null | $(call FLAG_IS_NAMED,$(1),$(2)); echo $$?)

override CC_FLAG = $(if $(filter 1,$(call CC_HAS_COMPILE_FLAG,$(1),$(2))),$(1))
override CXX_FLAG = $(if $(filter 1,$(call CXX_HAS_COMPILE_FLAG,$(1),$(2))),$(1))

override CXX_HAS_LINK_FLAG = $(shell LC_ALL=C $(CXX) $(CXXFLAGS) $(LDFLAGS) $(1) -Wl,--version 2>&1 >/dev/null | $(call FLAG_IS_NAMED,$(1),$(2)); echo $$?)

override LD_HAS_FLAG = $(shell ! LC_ALL=C $(CXX) $(CXXFLAGS) $(LDFLAGS) -Wl,--help 2>/dev/null | grep -qE '(^|[[:space:]])-?$(1)([[:space:],=[]|$$)'; echo $$?)

override SHARED_FLAGS := \
    -ffreestanding \
    -fno-common \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-PIC \
    -ffunction-sections \
    -fdata-sections

override CFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu11

override CXXFLAGS += \
    -Wall \
    -Wextra \
    -std=c++23 \
    -fno-rtti \
    -fno-exceptions

override CPPFLAGS_C := \
    -nostdinc \
    -I src \
    -I limine-protocol/include \
    -isystem freestanding-c-hdrs/x86_64/include \
    $(CPPFLAGS) \
    -MMD \
    -MP

override CPPFLAGS_CXX := \
    -nostdinc \
    -I src \
    -I limine-protocol/include \
    -isystem freestanding-cxx-hdrs/include \
    -isystem freestanding-c-hdrs/x86_64/include \
    $(CPPFLAGS) \
    -MMD \
    -MP \
    $(CPPFLAGS_C) 

    override NASMFLAGS := \
        $(patsubst -g,-g -F dwarf,$(NASMFLAGS)) \
        -i src/ \
        -Wall

override LDFLAGS := \
    -Wl,--build-id=sha1 \
    $(LDFLAGS)

ifeq ($(CC_IS_CLANG),1)
    override CC += \
        -target x86_64-unknown-none-elf
endif
ifeq ($(CXX_IS_CLANG),1)
    override CXX += \
        -target x86_64-unknown-none-elf

    override LDFLAGS := \
        -fuse-ld=lld \
        $(LDFLAGS)
endif

override LDFLAGS += \
    -nostdlib \
    -static \
    -Wl,-z,max-page-size=0x1000 \
    -Wl,-z,noexecstack \
    -Wl,--gc-sections \
    -Wl,-T,link.lds

    override SHARED_FLAGS += \
        -m64 \
        -march=x86-64 \
        -mno-mmx \
        -mno-sse \
        -mno-red-zone \
        -mcmodel=kernel
    override LDFLAGS += \
        -Wl,-m,elf_x86_64
    override NASMFLAGS := \
        -f elf64 \
        $(NASMFLAGS)


override CFLAGS += \
    $(SHARED_FLAGS)
override CXXFLAGS += \
    $(SHARED_FLAGS)

override CC_COMPILE_ERROR := $(shell LC_ALL=C $(CC) $(CFLAGS) -c -x c /dev/null -o /dev/null 2>&1 >/dev/null | grep -i 'error:' | head -n 1)
ifneq ($(CC_COMPILE_ERROR),)
    $(call TOOLCHAIN_ERROR,The C compiler does not work with the given CFLAGS: $(CC_COMPILE_ERROR))
endif
override CXX_COMPILE_ERROR := $(shell LC_ALL=C $(CXX) $(CXXFLAGS) -c -x c++ /dev/null -o /dev/null 2>&1 >/dev/null | grep -i 'error:' | head -n 1)
ifneq ($(CXX_COMPILE_ERROR),)
    $(call TOOLCHAIN_ERROR,The C++ compiler does not work with the given CXXFLAGS: $(CXX_COMPILE_ERROR))
endif

ifneq ($(CC_IS_CLANG),1)
    override CC_EMITS_ELF := $(shell ! $(CC) $(CFLAGS) -dM -E -x c /dev/null 2>/dev/null | grep -q 'define __ELF__ 1'; echo $$?)
    ifneq ($(CC_EMITS_ELF),1)
        $(call TOOLCHAIN_ERROR,The C compiler does not emit ELF objects)
    endif
endif
ifneq ($(CXX_IS_CLANG),1)
    override CXX_EMITS_ELF := $(shell ! $(CXX) $(CXXFLAGS) -dM -E -x c++ /dev/null 2>/dev/null | grep -q 'define __ELF__ 1'; echo $$?)
    ifneq ($(CXX_EMITS_ELF),1)
        $(call TOOLCHAIN_ERROR,The C++ compiler does not emit ELF objects)
    endif
endif

ifeq ($(CXX_IS_CLANG),1)
    override CLANG_PRINT_COMMANDS := -\#\#\#
    override CXX_LINKS_VIA_DRIVER = $(shell ! $(CXX) $(CXXFLAGS) $(LDFLAGS) -Wl,--version $(CLANG_PRINT_COMMANDS) 2>&1 | grep '^ "' | tail -n 1 | grep -q -- '-Wl,'; echo $$?)

    ifeq ($(CXX_LINKS_VIA_DRIVER),1)
        override LDFLAGS += \
            -target x86_64-linux-gnu
        ifeq ($(call CXX_HAS_COMPILE_FLAG,--no-default-config),1)
            override LDFLAGS += \
                --no-default-config
        endif

        ifeq ($(CXX_LINKS_VIA_DRIVER),1)
            $(call TOOLCHAIN_ERROR,The C++ compiler still hands the link over to another compiler driver)
        endif
    endif
endif

override CXX_LINK_ERROR := $(shell LC_ALL=C $(CXX) $(CXXFLAGS) $(LDFLAGS) -Wl,--version 2>&1 >/dev/null | grep -i 'error:' | head -n 1)
ifneq ($(CXX_LINK_ERROR),)
    $(call TOOLCHAIN_ERROR,The linker does not work with the given LDFLAGS: $(CXX_LINK_ERROR))
endif

override CFLAGS += $(call CC_FLAG,-fno-stack-clash-protection)
override CXXFLAGS += $(call CXX_FLAG,-fno-stack-clash-protection)
    override CFLAGS += $(call CC_FLAG,-fcf-protection=none)
    override CXXFLAGS += $(call CXX_FLAG,-fcf-protection=none)

    override CFLAGS += $(call CC_FLAG,-mno-80387)
    override CXXFLAGS += $(call CXX_FLAG,-mno-80387)

ifeq ($(call CXX_HAS_LINK_FLAG,-no-pie),1)
    override LDFLAGS += \
        -no-pie
endif

ifeq ($(call LD_HAS_FLAG,-no-pie),1)
    override LDFLAGS += \
        -Wl,-no-pie
endif

override SRCFILES := $(shell find -L src cc-runtime/src -type f 2>/dev/null | LC_ALL=C sort)

override CFILES := $(filter %.c,$(SRCFILES))
override CXXFILES := $(filter %.cpp,$(SRCFILES))
override ASFILES := $(filter %.S,$(SRCFILES))
override OBJ := $(addprefix obj-x86_64/,$(CFILES:.c=.c.o) $(CXXFILES:.cpp=.cpp.o) $(ASFILES:.S=.S.o))
override HEADER_DEPS := $(addprefix obj-x86_64/,$(CFILES:.c=.c.d) $(CXXFILES:.cpp=.cpp.d) $(ASFILES:.S=.S.d))

override NASMFILES := $(filter %.asm,$(SRCFILES))
override OBJ += $(addprefix obj-x86_64/,$(NASMFILES:.asm=.asm.o))
override HEADER_DEPS += $(addprefix obj-x86_64/,$(NASMFILES:.asm=.asm.d))


.PHONY: all
all: bin-x86_64/$(OUTPUT)

-include $(HEADER_DEPS)

bin-x86_64/$(OUTPUT): Makefile link.lds $(OBJ)
	mkdir -p "$(dir $@)"
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJ) -o $@

obj-x86_64/src/lib/memory.cpp.o: override CXXFLAGS += -fno-lto
obj-x86_64/cc-runtime/src/cc-runtime.c.o: override CFLAGS += -fno-lto

obj-x86_64/%.c.o: %.c Makefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS_C) -c $< -o $@

obj-x86_64/%.cpp.o: %.cpp Makefile
	mkdir -p "$(dir $@)"
	$(CXX) $(CXXFLAGS) $(CPPFLAGS_CXX) -c $< -o $@

obj-x86_64/%.S.o: %.S Makefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS_C) -c $< -o $@

obj-x86_64/%.asm.o: %.asm Makefile
	mkdir -p "$(dir $@)"
	nasm $(NASMFLAGS) -MD $(@:.o=.d) -MP $< -o $@


.PHONY: clean
clean:
	rm -rf bin-x86_64 obj-x86_64

.PHONY: distclean
distclean:
	rm -rf bin-* obj-* .deps-obtained .cache compile_commands.json freestanding-c-hdrs freestanding-cxx-hdrs cc-runtime limine-protocol

.PHONY: install
install: all
	install -d "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)"
	install -m 644 bin-x86_64/$(OUTPUT) "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)/$(OUTPUT)-x86_64"

.PHONY: uninstall
uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)/$(OUTPUT)-x86_64"
	-rmdir "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)"


DISTRO_ID := $(shell . /etc/os-release && echo $$ID)

# ==== COMPILERS ====
CC := i686-elf-gcc
AR := i686-elf-ar
AS := i686-elf-as

ifeq ($(DISTRO_ID),fedora)
    GRUB_MKRESCUE := grub2-mkrescue
else
    GRUB_MKRESCUE := grub-mkrescue
endif

# ==== FLAGS ====
WARNINGS := -Wall -Wextra
CFLAGS := -ffreestanding -O0
ARFLAGS := rcs

# ==== DIRECTORIES ====
LIBC_SRC_DIR := ./libc/src
LIBC_BIN_DIR := ./libc/bin
KRL_SRC_DIR := ./kernel/src
KRL_BIN_DIR := ./kernel/bin
KRL_BOOT_DIR := ./kernel/boot
ISO_DIR := ./isodir/boot/grub

# ==== OUTPUT ====
ELF := ./isodir/boot/justanos.elf
ISO := ./isodir/justanos.iso
LIBC_A := $(LIBC_BIN_DIR)/libc.a

# ==== SRC/OBJ LIST ====
KRL_SRCS := $(wildcard $(KRL_SRC_DIR)/*.s $(KRL_SRC_DIR)/*.c)
KRL_OBJS := $(addprefix $(KRL_BIN_DIR)/, $(addsuffix .o, $(basename $(notdir $(KRL_SRCS)))))

LIBC_SRCS := $(wildcard $(LIBC_SRC_DIR)/*.c)
LIBC_OBJS := $(patsubst $(LIBC_SRC_DIR)/%.c, $(LIBC_BIN_DIR)/%.o, $(LIBC_SRCS))


# === ALL ===
all: $(ISO)

# ==== LIBC ====
$(LIBC_BIN_DIR)/%.o: $(LIBC_SRC_DIR)/%.c | $(LIBC_BIN_DIR)
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS) $(WARNINGS)

$(LIBC_A): $(LIBC_OBJS)
	$(AR) $(ARFLAGS) $@ $^

# ==== KERNEL =====
$(KRL_BOOT_DIR)/boot.o: $(KRL_BOOT_DIR)/boot.s | $(KRL_BOOT_DIR)
	$(AS) $< -o $@

$(KRL_BIN_DIR)/%.o: $(KRL_SRC_DIR)/%.c | $(KRL_BIN_DIR)
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS) $(WARNINGS)

$(KRL_BIN_DIR)/%.o: $(KRL_SRC_DIR)/%.s | $(KRL_BIN_DIR)
	$(AS) $< -o $@

# ==== LINKER ====
$(ELF): $(KRL_BOOT_DIR)/boot.o $(KRL_OBJS) $(LIBC_A) | $(ISO_DIR)
	$(CC) -T ./kernel/linker.ld -o $@ $(CFLAGS) -nostdlib $^ -lgcc

# === ISO ===
$(ISO): $(ELF) | $(ISO_DIR)
	 $(GRUB_MKRESCUE) -o $@ isodir

# ==== DIRS ====
$(LIBC_BIN_DIR) $(KRL_BIN_DIR) $(KRL_BOOT_DIR) $(ISO_DIR):
	mkdir -p $@


# ==== CLEAN ====
clean:
	rm -f $(KRL_BOOT_DIR)/boot.o $(KRL_OBJS) $(LIBC_OBJS) $(LIBC_A) $(ISO) $(ELF)


# ==== RUN ====
run:
	qemu-system-i386 -d cpu_reset -cdrom $(ISO)


.PHONY: all clean run


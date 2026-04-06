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
CFLAGS := -ffreestanding -O0 -I./libc/include -I./kernel/include -I./drivers/include
ARFLAGS := rcs

# ==== DIRECTORIES ====
LIBC_SRC_DIR := ./libc/src
LIBC_BIN_DIR := ./libc/bin
KRL_SRC_DIR := ./kernel/src
KRL_BIN_DIR := ./kernel/bin
KRL_BOOT_DIR := ./kernel/boot
DRIVER_SRC_DIR := ./drivers/src
DRIVER_BIN_DIR := ./drivers/bin
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

DRIVER_SRCS := $(wildcard $(DRIVER_SRC_DIR)/*.c)
DRIVER_OBJS := $(patsubst $(DRIVER_SRC_DIR)/%.c, $(DRIVER_BIN_DIR)/%.o, $(DRIVER_SRCS))

# === ALL ===
all: $(ISO)

# ==== LIBC ====
$(LIBC_BIN_DIR)/%.o: $(LIBC_SRC_DIR)/%.c | $(LIBC_BIN_DIR)
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS) $(WARNINGS)

$(LIBC_A): $(LIBC_OBJS)
	$(AR) $(ARFLAGS) $@ $^

# ==== DRIVER ====
$(DRIVER_BIN_DIR)/%.o: $(DRIVER_SRC_DIR)/%.c | $(DRIVER_BIN_DIR)
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS) $(WARNINGS)

# ==== KERNEL ====
$(KRL_BOOT_DIR)/boot.o: $(KRL_BOOT_DIR)/boot.s | $(KRL_BOOT_DIR)
	$(AS) $< -o $@

$(KRL_BIN_DIR)/%.o: $(KRL_SRC_DIR)/%.c | $(KRL_BIN_DIR)
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS) $(WARNINGS)

$(KRL_BIN_DIR)/%.o: $(KRL_SRC_DIR)/%.s | $(KRL_BIN_DIR)
	$(AS) $< -o $@

# ==== LINKER ====
$(ELF): $(KRL_BOOT_DIR)/boot.o $(KRL_OBJS) $(DRIVER_OBJS) $(LIBC_A) | $(ISO_DIR)
	$(CC) -T ./kernel/linker.ld -o $@ $(CFLAGS) -nostdlib $^ -lgcc

# === ISO ===
$(ISO): $(ELF) | $(ISO_DIR)
	 $(GRUB_MKRESCUE) -o $@ isodir

# ==== DIRS ====
$(LIBC_BIN_DIR) $(DRIVER_BIN_DIR) $(KRL_BIN_DIR) $(KRL_BOOT_DIR) $(ISO_DIR):
	mkdir -p $@


# ==== CLEAN ====
clean:
	rm -f $(KRL_BOOT_DIR)/boot.o $(KRL_OBJS) $(DRIVER_OBJS) $(LIBC_OBJS) $(LIBC_A) $(ISO) $(ELF)


# ==== create TAP device ====
tap:
	sudo ip link add br0 type bridge
	sudo ip tuntap add tap0 mode tap
	sudo ip link set tap0 master br0
	sudo ip link set tap0 up
	sudo ip link set br0 up
	echo "MAKE: Network bridge br0 created"

# ==== RUN ====
run:
	qemu-system-i386 -netdev tap,id=net0,ifname=tap0,script=no,downscript=no -device e1000,netdev=net0 -cdrom $(ISO)


.PHONY: all clean tap run


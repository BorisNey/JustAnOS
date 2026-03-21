# ==== COMPILERS ====
CC := i686-elf-gcc
AR := i686-elf-ar
AS := i686-elf-as

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

# ==== OUTPUT ====
ISO := ./justanos.elf
LIBC_A := $(LIBC_BIN_DIR)/libc.a

# ==== SRC/OBJ LIST ====
KRL_SRCS := $(wildcard $(KRL_SRC_DIR)/*.s $(KRL_SRC_DIR)/*.c)
KRL_OBJS := $(addprefix $(KRL_BIN_DIR)/, $(addsuffix .o, $(basename $(notdir $(KRL_SRCS)))))

LIBC_SRCS := $(wildcard $(LIBC_SRC_DIR)/*.c)
LIBC_OBJS := $(patsubst $(LIBC_SRC_DIR)/%.c, $(LIBC_BIN_DIR)/%.o, $(LIBC_SRCS))


all: $(ISO) $(LIBC_A)

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
$(ISO): $(KRL_BOOT_DIR)/boot.o $(KRL_OBJS) $(LIBC_A)
	$(CC) -T ./linker.ld -o $@ $(CFLAGS) -nostdlib $^ -lgcc

# ==== DIRS ====
$(LIBC_BIN_DIR) $(KRL_BIN_DIR) $(KRL_BOOT_DIR):
	mkdir -p $@


# ==== CLEAN ====
clean:
	rm -f $(KRL_BOOT_DIR)/boot.o $(KRL_OBJS) $(LIBC_OBJS) $(ISO) $(LIBC_A)


# ==== RUN ====
run:
	qemu-system-i386 -d cpu_reset -kernel $(ISO)

.PHONY: all clean run


# ==== COMPILERS ====
CC := i686-elf-gcc
AR := i686-elf-ar
AS := i686-elf-as

# ==== FLAGS ====
WARNINGS := -Wall -Wextra
CFLAGS := -ffreestanding -O2
ARFLAGS := rcs

# ==== DIRECTORIES ====
LIBC_SRC_DIR := ./libc/src
LIBC_BIN_DIR := ./libc/bin
KRL_ASM_SRC_DIR := ./kernel/asm/src
KRL_ASM_BIN_DIR := ./kernel/asm/bin
KRL_C_SRC_DIR := ./kernel/c/src
KRL_C_BIN_DIR := ./kernel/c/bin

# ==== OUTPUT ====
ISO := ./myos.elf
LIBC_A := $(LIBC_BIN_DIR)/libc.a

# ==== SRC/OBJ LIST ====
KRL_ASM_SRCS := $(wildcard $(KRL_ASM_SRC_DIR)/*.s)
KRL_ASM_OBJS := $(patsubst $(KRL_ASM_SRC_DIR)/%.s, $(KRL_ASM_BIN_DIR)/%.o, $(KRL_ASM_SRCS))

#KRL_C_SRCS := $(wildcard $(KRL_C_SRC_DIR)/*.c)
KRL_C_SRCS := $(KRL_C_SRC_DIR)/kernel.c
KRL_C_OBJS := $(patsubst $(KRL_C_SRC_DIR)/%.c, $(KRL_C_BIN_DIR)/%.o, $(KRL_C_SRCS))

LIBC_SRCS := $(wildcard $(LIBC_SRC_DIR)/*.c)
LIBC_OBJS := $(patsubst $(LIBC_SRC_DIR)/%.c, $(LIBC_BIN_DIR)/%.o, $(LIBC_SRCS))

ALL_OBJS := $(KRL_ASM_OBJS) $(KRL_C_OBJS) $(LIBC_OBJS) $(ISO) $(LIBC_A)



all: $(ISO) $(LIBC_A)

# ==== LIBC ====
$(LIBC_BIN_DIR)/%.o: $(LIBC_SRC_DIR)/%.c | $(LIBC_BIN_DIR)
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS) $(WARNINGS)

$(LIBC_A): $(LIBC_OBJS)
	$(AR) $(ARFLAGS) $@ $^


# ==== KERNEL =====
$(KRL_C_OBJS): $(KRL_C_SRCS) | $(KRL_C_BIN_DIR)
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS) $(WARNINGS)

$(KRL_ASM_BIN_DIR)/%.o: $(KRL_ASM_SRC_DIR)/%.s | $(KRL_ASM_BIN_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)


# ==== LINKER ====
$(ISO): $(KRL_C_OBJS) $(KRL_ASM_OBJS) $(LIBC_A)
	$(CC) -T ./linker.ld -o $@ $(CFLAGS) -nostdlib $^ -lgcc


# ==== DIRS ====
$(LIBC_BIN_DIR) $(KRL_ASM_BIN_DIR) $(KRL_C_BIN_DIR):
	mkdir -p $@

# ==== CLEAN ====
clean:
	rm -f $(ALL_OBJS)

# ==== RUN ====
run:
	qemu-system-i386 -kernel myos.elf

.PHONY: all clean run

export
CROSS_  = riscv64-unknown-elf-
AR      = ${CROSS_}ar
CC      = $(CROSS_)gcc
LD      = $(CROSS_)ld
OBJCOPY = $(CROSS_)objcopy

TYPE    ?= Debug

ifeq ($(TYPE), Debug)
	TYPEFLAG = -g
else
	TYPEFLAG = -O3
endif

.PHONY: all run debug clean
all:
	@make -C kernel -s
	@make -C user -s

run: all
	@qemu-system-riscv64 -nographic -machine virt -kernel kernel/vmlinux -initrd user/hello.bin

debug: all
	@qemu-system-riscv64 -nographic -machine virt -kernel kernel/vmlinux -initrd user/hello.bin -S -s

clean:
	@make clean -C kernel -s
	@make clean -C user -s

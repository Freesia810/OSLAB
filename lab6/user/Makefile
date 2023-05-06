# This file is part of Zhejiang University Operating System Courses Project
# This program is under MIT license, see http://phvntom.tech/LICENSE.txt

ISA     ?= rv64imafd
ABI     ?= lp64
INCLUDE  = -I$(shell pwd)/include -I$(shell pwd)/arch/riscv/include
CF       = $(TYPEFLAG) -march=$(ISA) -mabi=$(ABI) -mcmodel=medany -ffunction-sections -fdata-sections -nostartfiles -nostdlib -nostdinc -static -lgcc -Wl,--nmagic -Wl,--gc-sections
CFLAG    = $(CF) $(INCLUDE) -DPRIORITY

ASM_SRC	 = $(sort $(wildcard *.S))
C_SRC    = $(sort $(wildcard *.c))
OBJ	     = $(patsubst %.S,%.o,$(ASM_SRC)) $(patsubst %.c,%.o,$(C_SRC))

all: hello.bin

%.o:%.c
	$(CC) $(CFLAG) -c $<

%.o:%.S
	$(CC) $(CFLAG)  -c $<

hello.bin: $(OBJ)
	$(CC) $(CFLAG) -T link.ld -o hello.elf $(OBJ)
	$(OBJCOPY) hello.elf -O binary hello.bin
	@echo "\033[32mMake user program Success! \033[0m"

clean:
	rm -rf *.o hello.*

CC_PREFIX=i686-elf-

SRCDIR=src
OBJDIR=obj
TESTDIR=test

CSRC= $(wildcard $(SRCDIR)/*.c)
ASSRC= $(wildcard $(SRCDIR)/*.s)

OBJS= $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(CSRC:.c=.o) $(ASSRC:.s=.o))

TARGET=kernel.bin
LDFLAGS= -T link.lds

GCC_HOST=gcc
GCC=$(CC_PREFIX)gcc
AS=$(CC_PREFIX)as
LD=$(CC_PREFIX)ld

CFLAGS=-std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS=-T linker.ld -lgcc -ffreestanding -O2 -nostdlib

QEMU=qemu-system-i386

$(shell mkdir -p $(OBJDIR))

all: $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	$(AS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(GCC) -c $^ -o $@ $(CFLAGS)

$(TARGET): $(OBJS)
	$(GCC) $(LDFLAGS) -o $@ $^

run: $(TARGET)
	$(QEMU) -kernel $(TARGET)

clean:
	rm -rf $(OBJDIR)
	rm $(TARGET)

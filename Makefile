CC_PREFIX=i686-elf-

SRCDIR=src
OBJDIR=obj
TESTDIR=test

ASM_SUFFIX=s

CSRC= $(wildcard $(SRCDIR)/*.c)
ASMSRC= $(wildcard $(SRCDIR)/*.$(ASM_SUFFIX))

OBJS= $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(CSRC:.c=.o) $(ASMSRC:.$(ASM_SUFFIX)=.o))

TARGET=kernel.bin
LDFLAGS= -T link.lds

GCC_HOST=gcc
GCC=$(CC_PREFIX)gcc
ASM=$(CC_PREFIX)as
LD=$(CC_PREFIX)ld

CFLAGS=-std=gnu99 -ffreestanding -O2 -Wall -Wextra
ASMFLAGS=
LDFLAGS=-T linker.ld -lgcc -ffreestanding -O2 -nostdlib

QEMU=qemu-system-i386

$(shell mkdir -p $(OBJDIR))

all: $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.$(ASM_SUFFIX)
	$(ASM) $(ASMFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(GCC) -c $^ -o $@ $(CFLAGS)

$(TARGET): $(OBJS)
	$(GCC) $(LDFLAGS) -o $@ $^

run: $(TARGET)
	$(QEMU) -kernel $(TARGET)

clean:
	rm -rf $(OBJDIR)
	rm $(TARGET)

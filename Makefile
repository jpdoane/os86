CC_PREFIX=i686-elf-

SRCDIR=src
BUILD_DIR=build
OBJDIR=$(BUILD_DIR)/obj
IMAGE_DIR=image
DEBUG_DIR=debug

ASM_SUFFIX=S
ASM_OBJ_SUFFIX=asm.o

CSRC= $(wildcard $(SRCDIR)/*.c)
ASMSRC= $(wildcard $(SRCDIR)/*.$(ASM_SUFFIX))

OBJS= $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(CSRC:.c=.o) $(ASMSRC:.$(ASM_SUFFIX)=.$(ASM_OBJ_SUFFIX)))


TARGET=$(BUILD_DIR)/kernel
ISO_IMG=$(BUILD_DIR)/kernel.iso
SYMBOLFILE=$(DEBUG_DIR)/sym.txt
LDSCRIPT = $(BUILD_DIR)/linker.ld

GCC_HOST=gcc
GCC=$(CC_PREFIX)gcc
ASM=$(CC_PREFIX)gcc
LD=$(CC_PREFIX)ld


OPTFLAGS=-O0
# CFLAGS=-std=gnu99 -ffreestanding  -Wall -Wextra -g $(OPTFLAGS)
CFLAGS=-ffreestanding  -Wall -Wextra -g $(OPTFLAGS)
ASMFLAGS= -DASM_FILE -masm=intel 
LDFLAGS=-T $(LDSCRIPT) -lgcc -ffreestanding -nostdlib

QEMU=qemu-system-i386

$(shell mkdir -p $(OBJDIR))

all: $(TARGET)

$(OBJDIR)/%.$(ASM_OBJ_SUFFIX): $(SRCDIR)/%.$(ASM_SUFFIX)
	$(ASM) -c $(ASMFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(GCC) -c $^ -o $@ $(CFLAGS)

$(TARGET): $(OBJS) $(LDSCRIPT)
	$(GCC) $(LDFLAGS) -o $@ $(OBJS)
	make list

$(ISO_IMG): $(TARGET) $(IMAGE_DIR)/boot/grub/grub.cfg
	cp $(TARGET) $(IMAGE_DIR)/boot/
	# genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o $(ISO_IMG) boot
	grub-mkrescue -o $(ISO_IMG) $(IMAGE_DIR)

$(SYMBOLFILE): $(TARGET)
	i686-elf-objcopy --only-keep-debug $(TARGET) $(DEBUG_DIR)/kern.sym
	nm $(DEBUG_DIR)/kern.sym | awk '{ print $$1" "$$3 }' > $(SYMBOLFILE)

list: $(TARGET)
	i686-elf-objdump $(TARGET) -S > $(BUILD_DIR)/kernel.list


debug: debug-qemu

debug-qemu: $(TARGET)
	$(QEMU) -s -S -kernel $(TARGET) &
	gdb -x debug/gdb.start

debug-asm: $(TARGET)
	$(QEMU) -s -S -kernel $(TARGET) &
	gdb -x debug/gdb_asm.start
	echo "done"

debug-bochs: $(ISO_IMG) $(SYMBOLFILE)
	bochs -f $(DEBUG_DIR)/bochsrc.txt

run: $(TARGET)
	$(QEMU) -kernel $(TARGET)


clean:
	-rm -rf $(OBJDIR)
	-rm $(DEBUG_DIR)/kern.sym
	-rm $(TARGET)
	-rm $(SYMBOLFILE)
	-rm $(ISO_IMG)

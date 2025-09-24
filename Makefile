# Source code files
IMAGDEF = image_def
SRCS = startup.c syscalls.c blink.c
APP  = firmware
LNKSCRIPT = link.ld

# Directory to create temporary build files in
BUILDDIR = build

# Compilation related variables
TOOLCHAIN = arm-none-eabi-
CFLAGS ?= -mcpu=cortex-m33 -mthumb -O2
LDFLAGS ?= -T $(LNKSCRIPT) -nostdlib

# Object files
OBJS = $(patsubst %.c,$(BUILDDIR)/%.o,$(SRCS))

build: makeDir $(BUILDDIR)/$(APP).bin $(BUILDDIR)/$(APP).uf2 copyUF2

makeDir:
	mkdir -p $(BUILDDIR)

# Compile each source file to an object file
$(BUILDDIR)/%.o: %.c
	$(TOOLCHAIN)gcc $(CFLAGS) -c $< -o $@

# Link object files to ELF, then generate BIN
$(BUILDDIR)/$(APP).bin: $(OBJS)
	$(TOOLCHAIN)gcc $(OBJS) $(IMAGDEF).s $(CFLAGS) $(LDFLAGS) -o $(BUILDDIR)/$(APP).elf
	$(TOOLCHAIN)objdump -hSD $(BUILDDIR)/$(APP).elf > $(BUILDDIR)/$(APP).objdump
	$(TOOLCHAIN)objcopy -O binary $(BUILDDIR)/$(APP).elf $@

# Convert BIN to UF2
$(BUILDDIR)/$(APP).uf2: $(BUILDDIR)/$(APP).bin
	picotool uf2 convert $< $@

copyUF2: $(BUILDDIR)/$(APP).uf2
	cp $(BUILDDIR)/$(APP).uf2 ./$(APP).uf2

clean:
	rm -rf $(BUILDDIR) $(APP).uf2

run: build $(APP).uf2
	picotool load $(APP).uf2
	picotool reboot

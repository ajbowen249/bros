
#CC65_DIR := cc65
CC65_DIR := "/c/Program Files/cc65"

CC := $(CC65_DIR)/bin/cc65
CA := $(CC65_DIR)/bin/ca65
LD := $(CC65_DIR)/bin/ld65

CC_ANNOTATE := python ./annotateld65/annotatecc65.py
LD_ANNOTATE := python ./annotateld65/annotateld65.py

TARGET_PLATFORM := nes

TARGETS := bros.nes

EMULATOR ?= higan

OBJECTS := $(TARGETS:.nes=.o)
ASSEMBLY_SOURCES := $(TARGETS:.nes=.s)

.PHONY: all

# Disable builtin rules (for .c, .o) by providing an empty .SUFFIXES rule
# Yes. GNU make is a rat's nest
.SUFFIXES:

# Make sure intermediate files are *NOT* deleted
# Yes. GNU make is a rat's nest
.PRECIOUS: %.s %.o

all: $(TARGETS)

# submodule build
$(CC):
	cd $(CC65_DIR) && $(MAKE)

%: %.nes
	$(EMULATOR) $<

clean:
	@rm -fv $(TARGETS)
	@rm -fv $(OBJECTS)
	@rm -fv $(ASSEMBLY_SOURCES)
	@rm -fv crt0.o
	@rm -fv .annotate.*
	@rm -fv *.nl
	@rm -fv debugSymbols

%.s: %.c
	$(CC) -o .annotate.bros.s -Oi $< --target $(TARGET_PLATFORM) -I$(CC65_DIR)/include/ --add-source
	$(CC_ANNOTATE)

%.o: %.s
	$(CA) $<

%.nes: %.o crt0.o
	$(LD) -Ln debugSymbols -C nrom_128_horz.cfg -o $@ $^ $(TARGET_PLATFORM).lib
	$(LD_ANNOTATE) -Ln debugSymbols -C nrom_128_horz.cfg -o $@ $^ $(TARGET_PLATFORM).lib

# Assumes fceux is in PATH
run:
	fceux bros.nes

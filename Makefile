
#CC65_DIR := cc65
CC65_DIR := "/c/Program Files/cc65"

CC := $(CC65_DIR)/bin/cc65
CA := $(CC65_DIR)/bin/ca65
LD := $(CC65_DIR)/bin/ld65

CC_ANNOTATE := python ./annotateld65/annotatecc65.py
LD_ANNOTATE := python ./annotateld65/annotateld65.py

INC_DIR := ./inc
SRC_DIR := ./src

C_SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
ASM_SRC_FILES := $(wildcard $(SRC_DIR)/*.s)

TARGET_PLATFORM := nes

EMULATOR := fceux

BUILD_DIR := ./build

ASM_ODIR := $(BUILD_DIR)
OBJ_ODIR := $(BUILD_DIR)
COMPILED_ASM := $(patsubst $(SRC_DIR)/%, $(ASM_ODIR)/%, $(C_SRC_FILES:.c=.s))
C_OBJECT_FILES := $(COMPILED_ASM:.s=.o)
ASM_OBJECT_FILES := $(OBJ_ODIR)/crt0.o

TARGET := $(BUILD_DIR)/bros.nes

.PHONY: all

# Disable builtin rules (for .c, .o) by providing an empty .SUFFIXES rule
.SUFFIXES:

# Make sure intermediate files are not deleted
.PRECIOUS: $(BUILD_DIR)/%.s $(BUILD_DIR)/%.o

all: $(TARGET)

$(ASM_ODIR)/%.s: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) -o ./$@ -Oi $< --target $(TARGET_PLATFORM) -I$(CC65_DIR)/include/ -I$(INC_DIR) --add-source

$(OBJ_ODIR)/%.o: $(ASM_ODIR)/%.s
	@mkdir -p $(BUILD_DIR)
	$(CA) $<

$(OBJ_ODIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(BUILD_DIR)
	$(CA) -o ./$@ $<

$(BUILD_DIR)/%.nes: $(C_OBJECT_FILES) $(ASM_OBJECT_FILES)
	@mkdir -p $(BUILD_DIR)
	$(LD) -Ln $(BUILD_DIR)/debugSymbols -C nrom_128_horz.cfg -o $@ $^ $(TARGET_PLATFORM).lib

clean:
	@rm -rfv $(BUILD_DIR)

run:
	$(EMULATOR) $(TARGET)

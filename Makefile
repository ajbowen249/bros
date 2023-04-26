
#CC65_DIR := cc65
CC65_DIR := "/c/Program Files/cc65"

# CC := $(CC65_DIR)/bin/cc65
# CA := $(CC65_DIR)/bin/ca65
# LD := $(CC65_DIR)/bin/ld65

CC := cc65
CA := ca65
LD := ld65

CC_ANNOTATE := python ./annotateld65/annotatecc65.py
LD_ANNOTATE := python ./annotateld65/annotateld65.py

PROCESS_APP_ASM := python ./applicationTools/process_asm.py
INSERT_SYMBOLS := python ./applicationTools/add_symbols_to_app_config.py

INC_DIR := ./inc
SRC_DIR := ./src
APP_SRC_DIR := $(SRC_DIR)/applications

C_SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
C_APP_FILES := $(wildcard $(APP_SRC_DIR)/*.c)
ASM_SRC_FILES := $(wildcard $(SRC_DIR)/*.s)

TARGET_PLATFORM := nes

EMULATOR := fceux

BUILD_DIR := ./build

ASM_ODIR := $(BUILD_DIR)
OBJ_ODIR := $(BUILD_DIR)
COMPILED_ASM := $(patsubst $(SRC_DIR)/%, $(ASM_ODIR)/%, $(C_SRC_FILES:.c=.s))
COMPILED_APP_ASM := $(patsubst $(APP_SRC_DIR)/%, $(ASM_ODIR)/%, $(C_APP_FILES:.c=.s.app))
COMPILED_APP_OBJ :=$(COMPILED_APP_ASM:.s.app=.o.app)
APP_BINARY_FILES :=$(COMPILED_APP_OBJ:.o.app=.nes.app)
C_OBJECT_FILES := $(COMPILED_ASM:.s=.o)
ASM_OBJECT_FILES := $(OBJ_ODIR)/crt0.o

SOURCE_APP_CONFIG_FILE := ./app_rom.cfg
GENERATED_APP_CONFIG_FILE := $(BUILD_DIR)/generated_app_config.cfg

KERNEL := $(BUILD_DIR)/bros.nes

.PHONY: all

# Disable builtin rules (for .c, .o) by providing an empty .SUFFIXES rule
.SUFFIXES:

# Make sure intermediate files are not deleted
.PRECIOUS: $(BUILD_DIR)/%.s $(BUILD_DIR)/%.s.app $(BUILD_DIR)/%.o $(BUILD_DIR)/%.o.app

all: $(KERNEL) $(APP_BINARY_FILES)

# Kernel build. This makes the actual NES ROM file

# Kernel C files
$(ASM_ODIR)/%.s: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) -o ./$(patsubst %.s,%.annotate.s,$@) -Oi $< --target $(TARGET_PLATFORM) -I$(CC65_DIR)/include/ -I$(INC_DIR) --add-source
	$(CC_ANNOTATE) ./$@

# Kernel ASM from C
$(OBJ_ODIR)/%.o: $(ASM_ODIR)/%.s
	@mkdir -p $(BUILD_DIR)
	$(CA) $<

# Kernel ASM
$(OBJ_ODIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(BUILD_DIR)
	$(CA) -o ./$@ $<

# Final NES file
$(BUILD_DIR)/%.nes: $(C_OBJECT_FILES) $(ASM_OBJECT_FILES)
	@mkdir -p $(BUILD_DIR)
	$(LD) -Ln $(BUILD_DIR)/debugSymbols -C kernel_rom.cfg -o $@ $^ $(TARGET_PLATFORM).lib
	$(LD_ANNOTATE) -Ln $(BUILD_DIR)/debugSymbols -C kernel_rom.cfg -o $@ $^ $(TARGET_PLATFORM).lib
	$(INSERT_SYMBOLS) $(BUILD_DIR)/debugSymbols $(SOURCE_APP_CONFIG_FILE) $(GENERATED_APP_CONFIG_FILE)

# Application build. These are compiled to BrOS application binaries with a special header to help the OS relocate them

# Application C files
$(ASM_ODIR)/%.s.app: $(APP_SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -Oi $< --target $(TARGET_PLATFORM) -I$(CC65_DIR)/include/ -I$(INC_DIR)
	$(PROCESS_APP_ASM) $@

# Application object files
$(OBJ_ODIR)/%.o.app: $(ASM_ODIR)/%.s.app
	@mkdir -p $(BUILD_DIR)
	$(warn "HERE" $<)
	$(CA) -o $@ $<

# Application binaries
# $(BUILD_DIR)/%.nes.app: $(C_OBJECT_FILES) $(ASM_OBJECT_FILES) $(OBJ_ODIR)/%.o.app
$(BUILD_DIR)/%.nes.app: $(OBJ_ODIR)/%.o.app
	@mkdir -p $(BUILD_DIR)
# Not quite working yet. Most things from the app seem to be making it to the correct symbol location, but somewhere
# Things that need to be fixed in memory are getting moved around.
	$(LD) --allow-multiple-definition -Ln ./$(patsubst %.nes.app,%.symbols,$@) -C $(GENERATED_APP_CONFIG_FILE) -o $@ $^ $(TARGET_PLATFORM).lib

clean:
	@rm -rfv $(BUILD_DIR)

run: $(KERNEL)
	$(EMULATOR) $<

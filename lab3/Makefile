# ECE3849 Makefile for TM4C1294 (Tiva C Series)

# Project settings
TARGET     := lab3
BUILD_DIR  := build

# Serial monitor settings
DEVICE     ?= /dev/ttyACM2
BAUD       ?= 115200

# Toolchain paths
TIVA       := $(HOME)/ti/sw-tm4c-2.2.0.295
CCS        := $(HOME)/ti/ccs1281/ccs
CGT        := $(CCS)/tools/compiler/ti-cgt-arm_20.2.7.LTS
CC         := $(CGT)/bin/armcl
CCSLIB     := $(CGT)/lib
CCSINC     := $(CGT)/include

# Source directories
SRC_DIR    := src
LIBS_DIR   := libs
LINKER_DIR := linker

# Include paths
INCLUDES := \
	--include_path=$(TIVA) \
	--include_path=$(SRC_DIR) \
	--include_path=$(LIBS_DIR)/HAL_TM4C1294 \
	--include_path=$(LIBS_DIR)/buttonsDriver \
	--include_path=$(LIBS_DIR)/display \
	--include_path=$(LIBS_DIR)/elapsedTime \
	--include_path=$(LIBS_DIR)/joystickDriver \
	--include_path=$(LIBS_DIR)/pll \
	--include_path=$(LIBS_DIR)/timerLib \
	--include_path=$(LIBS_DIR)/FreeRTOS \
	--include_path=$(LIBS_DIR)/FreeRTOS/FreeRTOS/include \
	--include_path=$(LIBS_DIR)/FreeRTOS/FreeRTOS/portable/CCS/ARM_CM4F \
	--include_path=$(CCSINC)

# Compiler flags
CFLAGS := \
	-mv7M4 \
	--code_state=16 \
	--float_support=FPv4SPD16 \
	-me \
	-O1 \
	-g \
	--gcc \
	--diag_warning=225 \
	--abi=eabi \
	--define=ccs \
	--define=PART_TM4C1294NCPDT \
	--define=TARGET_IS_TM4C129_RA1 \
	$(INCLUDES)

# Linker flags
LDFLAGS := \
	-mv7M4 \
	--code_state=16 \
	--float_support=FPv4SPD16 \
	-me \
	-O1 \
	-g \
	--gcc \
	--diag_warning=225 \
	--abi=eabi \
	--define=ccs \
	--define=PART_TM4C1294NCPDT \
	--define=TARGET_IS_TM4C129_RA1 \
	-z \
	-m$(BUILD_DIR)/$(TARGET).map \
	--heap_size=0 \
	--stack_size=2048 \
	-i$(TIVA)/driverlib/ccs/Debug \
	-i$(CCSLIB) \
	-i$(CCSINC) \
	--reread_libs \
	--rom_model

# Libraries to link
LIBS := \
	-l$(TIVA)/driverlib/ccs/Debug/driverlib.lib \
	-l$(TIVA)/grlib/ccs/Debug/grlib.lib \
	-llibc.a

# Source files
C_SOURCES := \
	$(SRC_DIR)/startup_ccs.c \
	$(LIBS_DIR)/pll/sysctl_pll.c \
	$(LIBS_DIR)/display/HAL_EK_TM4C1294XL_Crystalfontz128x128_ST7735.c \
	$(LIBS_DIR)/display/Crystalfontz128x128_ST7735.c \
	$(LIBS_DIR)/FreeRTOS/FreeRTOS/tasks.c \
	$(LIBS_DIR)/FreeRTOS/FreeRTOS/queue.c \
	$(LIBS_DIR)/FreeRTOS/FreeRTOS/list.c \
	$(LIBS_DIR)/FreeRTOS/FreeRTOS/timers.c \
	$(LIBS_DIR)/FreeRTOS/FreeRTOS/event_groups.c \
	$(LIBS_DIR)/FreeRTOS/FreeRTOS/stream_buffer.c \
	$(LIBS_DIR)/FreeRTOS/FreeRTOS/portable/CCS/ARM_CM4F/port.c \
	$(LIBS_DIR)/FreeRTOS/FreeRTOS/portable/MemMang/heap_4.c

CPP_SOURCES := \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/display.cpp \
	$(SRC_DIR)/game.cpp \
	$(SRC_DIR)/buzzer.cpp \
	$(LIBS_DIR)/timerLib/timerLib.cpp \
	$(LIBS_DIR)/buttonsDriver/button.cpp \
	$(LIBS_DIR)/joystickDriver/joystick.cpp

ASM_SOURCES := \
	$(LIBS_DIR)/FreeRTOS/FreeRTOS/portable/CCS/ARM_CM4F/portasm.asm

# Derive object files from source basenames
OBJECTS := \
	$(patsubst %.c,$(BUILD_DIR)/%.obj,$(notdir $(C_SOURCES))) \
	$(patsubst %.cpp,$(BUILD_DIR)/%.obj,$(notdir $(CPP_SOURCES))) \
	$(patsubst %.asm,$(BUILD_DIR)/%.obj,$(notdir $(ASM_SOURCES)))

# vpath: tell make where to find sources by extension
vpath %.c   $(sort $(dir $(C_SOURCES)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))
vpath %.asm $(sort $(dir $(ASM_SOURCES)))

# Linker script
LINKER_SCRIPT := $(LINKER_DIR)/tm4c1294.cmd

# Output files
ELF    := $(BUILD_DIR)/$(TARGET).out
BIN    := $(BUILD_DIR)/$(TARGET).bin
MAP    := $(BUILD_DIR)/$(TARGET).map

# Default target
.PHONY: all
all: $(BIN)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Check toolchain exists
.PHONY: check-toolchain
check-toolchain:
	@if [ ! -f "$(CC)" ]; then \
		echo "Error: TI compiler not found at $(CC)"; \
		echo "Please install CCS to ~/ti/ccs1281/"; \
		exit 1; \
	fi
	@if [ ! -d "$(TIVA)" ]; then \
		echo "Error: TivaWare not found at $(TIVA)"; \
		echo "Please install TivaWare to ~/ti/sw-tm4c-2.2.0.295/"; \
		exit 1; \
	fi

# Binary creation
$(BIN): $(ELF)
	@echo "Creating binary..."
	@if command -v arm-none-eabi-objcopy >/dev/null 2>&1; then \
		arm-none-eabi-objcopy -O binary $< $@; \
	else \
		nix-shell -p gcc-arm-embedded --run "arm-none-eabi-objcopy -O binary '$<' '$@'"; \
	fi
	@echo ""
	@echo "Build complete!"
	@ls -lh $(ELF) $(BIN)

# Linking
$(ELF): $(OBJECTS) $(LINKER_SCRIPT) | check-toolchain
	@echo "Linking..."
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LINKER_SCRIPT) $(LIBS)

# Pattern rules
$(BUILD_DIR)/%.obj: %.c | $(BUILD_DIR) check-toolchain
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< --output_file=$@

$(BUILD_DIR)/%.obj: %.cpp | $(BUILD_DIR) check-toolchain
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< --output_file=$@

$(BUILD_DIR)/%.obj: %.asm | $(BUILD_DIR) check-toolchain
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< --output_file=$@

# Clean target
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(BUILD_DIR)/*.obj $(BUILD_DIR)/*.out $(BUILD_DIR)/*.bin $(BUILD_DIR)/*.map
	@echo "Clean complete"

# Full rebuild
.PHONY: rebuild
rebuild: clean all

# Show build info
.PHONY: info
info:
	@echo "Target:    $(TARGET)"
	@echo "Compiler:  $(CC)"
	@echo "TivaWare:  $(TIVA)"
	@echo "Build dir: $(BUILD_DIR)"
	@echo "Sources:   $(words $(C_SOURCES) $(CPP_SOURCES)) C/C++ files + $(words $(ASM_SOURCES)) ASM file(s)"
	@echo "Objects:   $(OBJECTS)"

.PHONY: upload
upload: $(BIN)
	@echo "Uploading to TM4C1294..."
	sudo lm4flash $(BIN)

# Serial monitor
.PHONY: monitor
monitor:
	@DEVICE_TO_USE="$(DEVICE)"; \
	if [ ! -e "$$DEVICE_TO_USE" ]; then \
		for dev in /dev/ttyACM0 /dev/ttyACM1 /dev/ttyACM2 /dev/ttyUSB0 /dev/ttyUSB1; do \
			if [ -e "$$dev" ]; then \
				DEVICE_TO_USE="$$dev"; \
				break; \
			fi; \
		done; \
	fi; \
	if [ ! -e "$$DEVICE_TO_USE" ]; then \
		echo "Error: No serial device found"; \
		echo "Available devices:"; \
		ls -la /dev/ttyACM* /dev/ttyUSB* 2>/dev/null || echo "  None found"; \
		echo ""; \
		echo "Make sure the board is connected and you have permissions:"; \
		echo "  sudo usermod -a -G dialout $$USER"; \
		exit 1; \
	fi; \
	echo "Connecting to $$DEVICE_TO_USE at $(BAUD) baud..."; \
	echo "Press Ctrl+C to exit"; \
	echo ""; \
	stty -F "$$DEVICE_TO_USE" $(BAUD) cs8 -cstopb -parenb -crtscts -ixon -ixoff raw -echo; \
	cat "$$DEVICE_TO_USE"

# Upload and then monitor
.PHONY: flash
flash: upload monitor

.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all      - Build the project (default)"
	@echo "  clean    - Remove build artifacts"
	@echo "  rebuild  - Clean and build"
	@echo "  upload   - Flash firmware to board"
	@echo "  monitor  - Open serial monitor (DEVICE=$(DEVICE) BAUD=$(BAUD))"
	@echo "  flash    - Upload and monitor"
	@echo "  info     - Show build configuration"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make upload                    # Flash using default settings"
	@echo "  make monitor DEVICE=/dev/ttyACM1 BAUD=9600"

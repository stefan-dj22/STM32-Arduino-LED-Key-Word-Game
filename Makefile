# Toolchain
TOOLCHAIN_PATH ?= $(shell which arm-none-eabi-gcc | sed 's|/bin/arm-none-eabi-gcc||')
ifeq ($(TOOLCHAIN_PATH),)
    $(error "arm-none-eabi-gcc not found in PATH. Please set TOOLCHAIN_PATH environment variable")
endif

CC = $(TOOLCHAIN_PATH)/bin/arm-none-eabi-gcc
CXX = $(TOOLCHAIN_PATH)/bin/arm-none-eabi-g++
AS = $(TOOLCHAIN_PATH)/bin/arm-none-eabi-as
LD = $(TOOLCHAIN_PATH)/bin/arm-none-eabi-ld
OBJCOPY = $(TOOLCHAIN_PATH)/bin/arm-none-eabi-objcopy
SIZE = $(TOOLCHAIN_PATH)/bin/arm-none-eabi-size
OBJDUMP = $(TOOLCHAIN_PATH)/bin/arm-none-eabi-objdump

# MCU name
MCU = -mcpu=cortex-m3 -mthumb

# Build flags
CFLAGS = $(MCU) -Wall -Wextra -g -O0 -ffunction-sections -fdata-sections
CFLAGS += -DUSE_HAL_DRIVER -DSTM32F100xB
CFLAGS += -I./Core/Inc -I./Drivers/STM32F1xx_HAL_Driver/Inc -I./Drivers/CMSIS/Device/ST/STM32F1xx/Include -I./Drivers/CMSIS/Include -I./Drivers/TM1638/Inc
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti

# Source files
SRCS = $(wildcard Core/Src/*.c)
SRCS += $(wildcard Drivers/STM32F1xx_HAL_Driver/Src/*.c)
SRCS += $(wildcard Drivers/TM1638/Src/*.c)
CPPSRCS = $(wildcard Core/Src/*.cpp)
CPPSRCS += $(wildcard Drivers/TM1638/Src/*.cpp)
ASMS = Core/Startup/startup_stm32f100rbtx.s

# Object files
OBJS = $(SRCS:.c=.o) $(CPPSRCS:.cpp=.o) $(ASMS:.s=.o)

# Target
TARGET = STM32-Arduino-LED-Key-Word-Game

# Default target
all: $(TARGET).elf

# Linking
$(TARGET).elf: $(OBJS)
	$(CXX) $(OBJS) $(CXXFLAGS) -TSTM32F100RBTX_FLASH.ld -o $@
	$(SIZE) $@
	$(OBJDUMP) -D $@ > $(TARGET).dis

# Compilation
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(MCU) -c $< -o $@

# Clean
clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).dis

.PHONY: all clean 
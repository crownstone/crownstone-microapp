#!/bin/make

GCC_PATH=$(HOME)/workspace/bluenet/tools/gcc_arm_none_eabi/bin

SHARED_PATH=$(HOME)/workspace/bluenet/source/shared

CC=$(GCC_PATH)/arm-none-eabi-gcc
OBJCOPY=$(GCC_PATH)/arm-none-eabi-objcopy
OBJDUMP=$(GCC_PATH)/arm-none-eabi-objdump
NM=$(GCC_PATH)/arm-none-eabi-nm

BUILD_PATH=build

TARGET=$(BUILD_PATH)/example

FLAGS=-mthumb -ffunction-sections -fdata-sections -Wall -Werror -fno-strict-aliasing -fno-builtin -fshort-enums -Wno-error=format -Wno-error=unused-function -Os -fomit-frame-pointer -Wl,-z,nocopyreloc -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -u _printf_float

MAIN_SYMBOL=dummy_main

all: init $(TARGET).hex
	echo "Result: $(TARGET).hex"

init: 
	mkdir -p $(BUILD_PATH)

$(TARGET).elf: src/main.c example.c $(SHARED_PATH)/ipc/cs_IpcRamData.c
	$(CC) $(FLAGS) $^ -I$(SHARED_PATH) -Iinclude -Linclude -Tgeneric_gcc_nrf52.ld -o $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $^ $@

flash: $(TARGET).hex
	nrfjprog -f nrf52 --program $(TARGET).hex --sectorerase

show_addresses: $(TARGET).elf
	echo -n "$(MAIN_SYMBOL)():\t"
	$(NM) $^ | grep -w $(MAIN_SYMBOL) | cut -f1 -d' '
	echo -n "setup():\t"
	$(NM) $^ | grep -w setup | cut -f1 -d' '
	echo -n "loop():\t\t"
	$(NM) $^ | grep -w loop | cut -f1 -d' '

inspect: $(TARGET).elf
	$(OBJDUMP) -d $^ | awk -F"\n" -v RS="\n\n" '$$1 ~ /<$(MAIN_SYMBOL)>/'

help:
	echo "make\t\t\tbuild .elf and .hex files (requires the ARM cross-compiler)"
	echo "make flash\t\tflash .hex file to target (requires nrfjprog)"
	echo "make show_addresses\tshow the addresses of $(MAIN_SYMBOL), setup, and loop functions"
	echo "make inspect\t\tdecompile the $(MAIN_SYMBOL) function"

.PHONY: flash show_addresses inspect help

.SILENT: all init flash show_addresses inspect help

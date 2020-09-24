#!/bin/make

GCC_PATH=$(HOME)/workspace/bluenet/tools/gcc_arm_none_eabi/bin

SHARED_PATH=$(HOME)/workspace/bluenet/source/shared

CC=$(GCC_PATH)/arm-none-eabi-gcc
OBJCOPY=$(GCC_PATH)/arm-none-eabi-objcopy
OBJDUMP=$(GCC_PATH)/arm-none-eabi-objdump
NM=$(GCC_PATH)/arm-none-eabi-nm
SIZE=$(GCC_PATH)/arm-none-eabi-size

BUILD_PATH=build

TARGET=$(BUILD_PATH)/example

START_ADDRESS_WITHOUT_PREFIX=68000

START_ADDRESS=0x$(START_ADDRESS_WITHOUT_PREFIX)

# With -fPIC we get a segfault, we need to be more careful
#RELOC_FLAGS=-fPIC
#-msingle-pic-base -mpic-register=r9 -mno-pic-data-is-text-relative

#RELOC_FLAGS=-fPIC -msingle-pic-base -mpic-register=r9 -mno-pic-data-is-text-relative

FLAGS=-mthumb -ffunction-sections -fdata-sections $(RELOC_FLAGS) -Wall -Werror -fno-strict-aliasing -fno-builtin -fshort-enums -Wno-error=format -Wno-error=unused-function -Os -fomit-frame-pointer -Wl,-z,nocopyreloc -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -u _printf_float

MAIN_SYMBOL=dummy_main
SETUP_SYMBOL=setup
LOOP_SYMBOL=loop

all: init $(TARGET).hex $(TARGET).bin
	echo "Result: $(TARGET).hex"

clean:
	rm $(TARGET).hex
	rm $(TARGET).bin
	rm $(TARGET).elf

init:
	mkdir -p $(BUILD_PATH)

$(TARGET).elf: src/main.c example.c $(SHARED_PATH)/ipc/cs_IpcRamData.c
	$(CC) $(FLAGS) $^ -I$(SHARED_PATH) -Iinclude -Linclude -Tgeneric_gcc_nrf52.ld -o $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $^ $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $^ $@

flash: $(TARGET).hex
	nrfjprog -f nrf52 --program $(TARGET).hex --sectorerase

read:
	nrfjprog -f nrf52 --memrd $(START_ADDRESS) --w 8 --n 400

download: init
	nrfjprog -f nrf52 --memrd $(START_ADDRESS) --w 16 --n 0x2000 | tr [:upper:] [:lower:] | cut -f2 -d':' | cut -f1 -d'|' > $(BUILD_PATH)/download.txt

dump: $(TARGET).bin
	hexdump -v $^ | cut -f2- -d' ' > $(TARGET).txt

compare: dump download
	meld $(TARGET).txt $(BUILD_PATH)/download.txt 

erase:
	nrfjprog --erasepage 0x68000-0x6A000

reset:
	nrfjprog --reset

show_addresses: $(TARGET).elf
	echo -n "$(MAIN_SYMBOL)():\t"
	$(NM) $^ | grep -w $(MAIN_SYMBOL) | cut -f1 -d' '
	echo -n "setup():\t"
	$(NM) $^ | grep -w setup | cut -f1 -d' '
	echo -n "loop():\t\t"
	$(NM) $^ | grep -w loop | cut -f1 -d' '

inspect-main: $(TARGET).elf
	$(OBJDUMP) -d $^ | awk -F"\n" -v RS="\n\n" '$$1 ~ /<$(MAIN_SYMBOL)>/'

inspect-setup: $(TARGET).elf
	$(OBJDUMP) -d $^ | awk -F"\n" -v RS="\n\n" '$$1 ~ /<$(SETUP_SYMBOL)>/'

inspect-loop: $(TARGET).elf
	$(OBJDUMP) -d $^ | awk -F"\n" -v RS="\n\n" '$$1 ~ /<$(LOOP_SYMBOL)>/'

inspect: $(TARGET).elf
	$(OBJDUMP) -x $^ 

offset: $(TARGET).elf
	$(OBJDUMP) -d $^ | awk -F"\n" -v RS="\n\n" '$$1 ~ /<$(MAIN_SYMBOL)>/' | head -n1 | cut -f1 -d' ' \
		| tr [:lower:] [:upper:] \
		| xargs -i echo "obase=16;ibase=16;{} - $(START_ADDRESS_WITHOUT_PREFIX)" | bc | xargs -i echo "Offset is 0x{}"

size: $(TARGET).elf
	$(SIZE) -B $^ | tail -n1 | tr '\t' ' ' | tr -s ' ' | sed 's/^ //g' | cut -f1,2 -d ' ' | tr ' ' '+' \
		| bc | xargs -i echo "Total size: {} B"
	$(SIZE) -B $^ | tail -n1 | tr '\t' ' ' | tr -s ' ' | sed 's/^ //g' | cut -f1 -d ' ' | tr ' ' '+' \
		| bc | xargs -i echo "     flash: {} B"
	$(SIZE) -B $^ | tail -n1 | tr '\t' ' ' | tr -s ' ' | sed 's/^ //g' | cut -f1 -d ' ' | tr ' ' '+' \
		| xargs -i echo "({} + 1023) / 1024" | bc | xargs -i echo "     pages: {}"

help:
	echo "make\t\t\tbuild .elf and .hex files (requires the ARM cross-compiler)"
	echo "make flash\t\tflash .hex file to target (requires nrfjprog)"
	echo "make show_addresses\tshow the addresses of $(MAIN_SYMBOL), setup, and loop functions"
	echo "make inspect\t\tobjdump everything"
	echo "make inspect-main\t\tdecompile the $(MAIN_SYMBOL) function"
	echo "make size\t\tshow size information"

.PHONY: flash show_addresses inspect help read reset erase offset

.SILENT: all init flash show_addresses inspect size help read reset erase offset

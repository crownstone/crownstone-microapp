#!/bin/make

include config.mk
-include private.mk

MAIN_SYMBOL=dummy_main
SETUP_SYMBOL=setup
LOOP_SYMBOL=loop

all: init $(TARGET).hex $(TARGET).bin
	echo "Result: $(TARGET).hex"

clean:
	rm -f $(TARGET).hex
	rm -f $(TARGET).bin
	rm -f $(TARGET).elf
	echo "Cleaned build directory"

init:
	mkdir -p $(BUILD_PATH)

$(TARGET).elf: src/main.c src/microapp.c src/Arduino.c src/Wire.cpp src/Serial.cpp $(TARGET).c $(SHARED_PATH)/ipc/cs_IpcRamData.c
	$(CC) $(FLAGS) $^ -I$(SHARED_PATH) -Iinclude -Linclude -Tgeneric_gcc_nrf52.ld -o $@

$(TARGET).c: $(TARGET_NAME).ino
	echo '#include <Arduino.h>' > $(TARGET).c
	cat $(TARGET_NAME).ino >> $(TARGET).c

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

ota-request:
	scripts/microapp.py $(KEYS_JSON) $(BLE_ADDRESS) $(TARGET).bin request

ota-upload:
	scripts/microapp.py $(KEYS_JSON) $(BLE_ADDRESS) $(TARGET).bin upload

ota-validate:
	scripts/microapp.py $(KEYS_JSON) $(BLE_ADDRESS) $(TARGET).bin validate

ota-enable:
	scripts/microapp.py $(KEYS_JSON) $(BLE_ADDRESS) $(TARGET).bin enable

checksum:
	scripts/inspect_microapp.py $(TARGET).bin

show_addresses: $(TARGET).elf
	echo -n "$(MAIN_SYMBOL)():\t"
	$(NM) $^ | grep -w $(MAIN_SYMBOL) | cut -f1 -d' '
	echo -n "setup():\t"
	$(NM) $^ | grep -w $(SETUP_SYMBOL) | cut -f1 -d' '
	echo -n "loop():\t\t"
	$(NM) $^ | grep -w $(LOOP_SYMBOL) | cut -f1 -d' '

inspect-main: $(TARGET).elf
	$(OBJDUMP) -d $^ | awk -F"\n" -v RS="\n\n" '$$1 ~ /<$(MAIN_SYMBOL)>/'

inspect: $(TARGET).elf
	$(OBJDUMP) -x $^

inspect-headers: $(TARGET).elf
	$(OBJDUMP) -h $^

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

.PHONY: flash show_addresses inspect help read reset erase offset all

.SILENT: all init flash show_addresses inspect size help read reset erase offset clean

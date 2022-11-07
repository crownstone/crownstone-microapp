#!/bin/make

# This version of the Makefile does not use setup, loop, or main symbols

# Adjust target config file for nrf52832 vs nrf52840
#TARGET_CONFIG_FILE=target_nrf52840.mk
TARGET_CONFIG_FILE=target_nrf52832.mk

include $(TARGET_CONFIG_FILE)
include config.mk
-include private.mk

SOURCE_FILES=include/startup.S src/main.c src/microapp.c src/Arduino.c src/Wire.cpp src/Serial.cpp src/ArduinoBLE.cpp src/BleUtils.cpp src/BleDevice.cpp src/BleScan.cpp src/BleService.cpp src/BleCharacteristic.cpp src/BleMacAddress.cpp src/BleUuid.cpp src/Mesh.cpp src/CrownstoneSwitch.cpp src/ServiceData.cpp src/PowerUsage.cpp src/Presence.cpp src/Message.cpp src/BluenetInternal.cpp $(SHARED_PATH)/ipc/cs_IpcRamData.c $(TARGET).c

# First initialize, then create .hex file, then .bin file and file end with info
all: init $(TARGET).hex $(TARGET).bin $(TARGET).info
	@echo "Result: $(TARGET).hex (and $(TARGET).bin)"

clean:
	@rm -f $(TARGET).*
	@rm -f include/microapp_symbols.ld
	@rm -f include/microapp_header_symbols.ld
	@echo "Cleaned build directory"

init: $(TARGET_CONFIG_FILE)
	@echo "Use file: $(TARGET_CONFIG_FILE)"
	@echo 'Create build directory'
	@mkdir -p $(BUILD_PATH)
	@rm -f include/microapp_header_symbols.ld

.PHONY:
include/microapp_header_symbols.ld: $(TARGET).bin.tmp
	@echo "Use python script to generate $@ file with valid header symbols"
	@scripts/microapp_make.py -i $^ $@

.PHONY:
include/microapp_header_dummy_symbols.ld:
	@echo "Use python script to generate file with dummy values"
	@scripts/microapp_make.py include/microapp_header_symbols.ld

.tmp.TARGET_CONFIG_FILE.$(TARGET_CONFIG_FILE):
	@rm -f .tmp.TARGET_CONFIG_FILE.*
	touch $@

include/microapp_target_symbols.ld: $(TARGET_CONFIG_FILE) .tmp.TARGET_CONFIG_FILE.$(TARGET_CONFIG_FILE)
	@echo 'This script requires the presence of "bc" on the command-line'
	@echo 'Generate target symbols (from .mk file to .ld file)'
	@echo '/* Auto-generated file */' > $@
	@echo "APPLICATION_START_ADDRESS = $(START_ADDRESS);" >> $@
	@echo '' >> $@
	@echo "RAM_END = $(RAM_END);" >> $@

include/microapp_symbols.ld: include/microapp_symbols.ld.in
	@echo "Generate linker symbols using C header files (using the compiler)"
	@$(CC) -CC -E -P -x c -Iinclude $^ -o $@
	@echo "File $@ now up to date"

$(TARGET).elf.tmp.deps: include/microapp_header_dummy_symbols.ld include/microapp_symbols.ld include/microapp_target_symbols.ld
	@echo "Dependencies for $(TARGET).elf.tmp fulfilled"

$(TARGET).elf.tmp: $(SOURCE_FILES)
	@echo "Compile without firmware header"
	@$(CC) $(FLAGS) $^ -I$(SHARED_PATH) -Iinclude -Linclude -Tgeneric_gcc_nrf52.ld -o $@

.ALWAYS:
$(TARGET).elf.deps: include/microapp_header_symbols.ld
	@echo "Run scripts"

$(TARGET).elf: $(SOURCE_FILES)
	@echo "Compile with firmware header"
	@$(CC) $(FLAGS) $^ -I$(SHARED_PATH) -Iinclude -Linclude -Tgeneric_gcc_nrf52.ld -o $@

$(TARGET).c: $(TARGET_SOURCE)
	@echo "Script from .ino file to .c file (just adding Arduino.h header)"
	@echo '#include <Arduino.h>' > $(TARGET).c
	@cat $(TARGET_SOURCE) >> $(TARGET).c

$(TARGET).hex: $(TARGET).elf.deps $(TARGET).elf
	@echo "Create hex file from elf file"
	@$(OBJCOPY) -O ihex $(TARGET).elf $@

$(TARGET).bin.tmp: $(TARGET).elf.tmp.deps $(TARGET).elf.tmp
	@echo "Create temporary bin file from temporary elf file"
	@$(OBJCOPY) -O binary $(TARGET).elf.tmp $@

$(TARGET).bin: $(TARGET).elf
	@echo "Create final binary file"
	@$(OBJCOPY) -O binary $(TARGET).elf $@

$(TARGET).info:
	@echo "$(shell cat include/microapp_header_symbols.ld)"

flash: all
	echo nrfjprog -f nrf52 --program $(TARGET).hex --sectorerase --verify
	nrfjprog -f nrf52 --program $(TARGET).hex --sectorerase --verify

read:
	nrfjprog -f nrf52 --memrd $(START_ADDRESS) --w 8 --n 400

download: init
	nrfjprog -f nrf52 --memrd $(START_ADDRESS) --w 16 --n 0x2000 | tr [:upper:] [:lower:] | cut -f2 -d':' | cut -f1 -d'|' > $(BUILD_PATH)/download.txt

dump: $(TARGET).bin
	hexdump -v $^ | cut -f2- -d' ' > $(TARGET).txt

compare: dump download
	meld $(TARGET).txt $(BUILD_PATH)/download.txt

erase:
	$(eval STOP_ADDRESS_WITHOUT_PREFIX=$(shell echo 'obase=16;ibase=16;$(START_ADDRESS_WITHOUT_PREFIX)+$(MICROAPP_PAGES)*1000' | bc))
	echo nrfjprog --erasepage $(START_ADDRESS)-0x$(STOP_ADDRESS_WITHOUT_PREFIX)
	nrfjprog --erasepage $(START_ADDRESS)-0x$(STOP_ADDRESS_WITHOUT_PREFIX)

reset:
	nrfjprog --reset

upload-ble:
	cs_microapp_upload --keyFile $(KEYS_JSON) -a $(BLE_ADDRESS) -f $(TARGET).bin
	
upload-uart:
	cs_microapp_upload --keyFile $(KEYS_JSON) -d $(UART_DEVICE) -l $(LOG_STRINGS_FILE) -f $(TARGET).bin

inspect: $(TARGET).elf
	$(OBJDUMP) -x $^

inspect-headers: $(TARGET).elf
	$(OBJDUMP) -h $^

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
	echo "make inspect\t\tobjdump everything"
	echo "make size\t\tshow size information"

.PHONY: flash inspect help read reset erase all

.SILENT: all init flash inspect size help read reset erase clean

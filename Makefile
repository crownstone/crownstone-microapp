#!/bin/make

# Adjust target config file for nrf52832 vs nrf52840
TARGET_CONFIG_FILE=target_nrf52840.mk

include $(TARGET_CONFIG_FILE)
include config.mk
-include private.mk

MAIN_SYMBOL=dummy_main
SETUP_SYMBOL=setup
LOOP_SYMBOL=loop

# First initialize, then create .hex file, then .bin file and file end with info
all: init $(TARGET).hex $(TARGET).bin $(TARGET).info
	@echo "Result: $(TARGET).hex (and $(TARGET).bin)"

clean:
	@rm -f $(TARGET).*
	@echo "Cleaned build directory"

init:
	@echo 'Create build directory'
	@mkdir -p $(BUILD_PATH)
	@rm -f include/microapp_header_symbols.ld

include/microapp_header_symbols.ld: $(TARGET).bin.tmp
	@echo "Use python script to generate file with valid header symbols"
	@scripts/microapp_make.py -i $^ $@

.PHONY:
include/microapp_header_dummy_symbols.ld:
	@echo "Use python script to generate file with dummy values"
	@scripts/microapp_make.py include/microapp_header_symbols.ld

include/microapp_target_symbols.ld: $(TARGET_CONFIG_FILE)
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

$(TARGET).elf.tmp: include/startup.S src/main.c src/microapp.c src/Arduino.c src/Wire.cpp src/Serial.cpp src/ArduinoBLE.cpp $(SHARED_PATH)/ipc/cs_IpcRamData.c $(TARGET).c
	@echo "Compile without firmware header"
	@$(CC) $(FLAGS) $^ -I$(SHARED_PATH) -Iinclude -Linclude -Tgeneric_gcc_nrf52.ld -o $@

.ALWAYS:
$(TARGET).elf.deps: include/microapp_header_symbols.ld
	@echo "Run scripts"

$(TARGET).elf: include/startup.S src/main.c src/microapp.c src/Arduino.c src/Wire.cpp src/Serial.cpp src/ArduinoBLE.cpp $(SHARED_PATH)/ipc/cs_IpcRamData.c $(TARGET).c
	@echo "Compile with firmware header"
	@$(CC) $(FLAGS) $^ -I$(SHARED_PATH) -Iinclude -Linclude -Tgeneric_gcc_nrf52.ld -o $@

$(TARGET).c: $(TARGET_NAME).ino
	@echo "Script from .ino file to .c file (just adding Arduino.h header)"
	@echo '#include <Arduino.h>' > $(TARGET).c
	@cat $(TARGET_NAME).ino >> $(TARGET).c

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
	echo nrfjprog -f nrf52 --program $(TARGET).hex --sectorerase
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
	$(eval STOP_ADDRESS_WITHOUT_PREFIX=$(shell echo 'obase=16;ibase=16;$(START_ADDRESS_WITHOUT_PREFIX)+$(MICROAPP_PAGES)*1000' | bc))
	echo nrfjprog --erasepage $(START_ADDRESS)-0x$(STOP_ADDRESS_WITHOUT_PREFIX)
	nrfjprog --erasepage $(START_ADDRESS)-0x$(STOP_ADDRESS_WITHOUT_PREFIX)

reset:
	nrfjprog --reset

ota-upload:
	scripts/upload_microapp.py --keyFile $(KEYS_JSON) -a $(BLE_ADDRESS) -f $(TARGET).bin

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

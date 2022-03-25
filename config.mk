###############################################################################
# The following variables might need adjusting on your local system. Change
# them here or in private.mk. The latter file can be created if it does not
# exist.
###############################################################################

# Path to the gcc arm none eabi compiler (do not use another one).
GCC_PATH=$(HOME)/workspace/bluenet/tools/gcc_arm_none_eabi/bin

# The path to the bluenet repository
BLUENET_PATH=$(HOME)/workspace/bluenet

###############################################################################
# The variables following this section probably do not need to be changed.
###############################################################################

# Shared path in the bluenet repos
SHARED_PATH=$(BLUENET_PATH)/source/shared

# The different gcc tools
CC=$(GCC_PATH)/arm-none-eabi-g++
OBJCOPY=$(GCC_PATH)/arm-none-eabi-objcopy
OBJDUMP=$(GCC_PATH)/arm-none-eabi-objdump
NM=$(GCC_PATH)/arm-none-eabi-nm
SIZE=$(GCC_PATH)/arm-none-eabi-size
STRIP=$(GCC_PATH)/arm-none-eabi-strip
READELF=$(GCC_PATH)/arm-none-eabi-readelf

# The build directory
BUILD_PATH=build

# The target name
TARGET_NAME=basic

#TARGET_NAME=ble_peripheral_xiaomi_thermometer

# The target source file
TARGET_SOURCE=examples/$(TARGET_NAME).ino

# The build target (including build directory)
TARGET=$(BUILD_PATH)/$(TARGET_NAME)

# Number of pages
MICROAPP_PAGES=2

# For c++
# I have removed nanolib as well. Now I have to check if for e.g. memset I
# have to provide a function call to the memset of Crownstone.
FLAGS=-std=c++17 -mthumb -ffunction-sections -fdata-sections -Wall -Werror \
	  -fno-strict-aliasing -fno-builtin -fshort-enums -Wno-error=format \
	  -fno-exceptions -fdelete-dead-exceptions -fno-unwind-tables -fno-non-call-exceptions \
	  -nostdlib -fno-threadsafe-statics \
	  -Wl,--gc-sections -Wl,-eReset_Handler \
	  -g \
	  -Wno-error=unused-function -Os -fomit-frame-pointer -Wl,-z,nocopyreloc --specs=nano.specs \
	  -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -u _printf_float

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

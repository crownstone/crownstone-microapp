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

# The build directory
BUILD_PATH=build

# The target name
TARGET_NAME=example

# The target itself (including build directory)
TARGET=$(BUILD_PATH)/$(TARGET_NAME)

# The location we will write into (in flash)
START_ADDRESS_WITHOUT_PREFIX=69000

# The location including 0x
START_ADDRESS=0x$(START_ADDRESS_WITHOUT_PREFIX)

# The flags we will use. If you change this, the compiled program very likely
# does not work... Only do this if you have experience with embedded 
# development, linking, memory management, etc.
FLAGS=-mthumb -ffunction-sections -fdata-sections -Wall -Werror \
	  -fno-strict-aliasing -fno-builtin -fshort-enums -Wno-error=format \
	  -Wno-error=unused-function -Os -fomit-frame-pointer -Wl,-z,nocopyreloc \
	  -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -u _printf_float


# For c++
# I have removed nanolib as well. Now I have to check if for e.g. memset I
# have to provide a function call to the memset of Crownstone.
FLAGS=-std=c++17 -mthumb -ffunction-sections -fdata-sections -Wall -Werror \
	  -fno-strict-aliasing -fno-builtin -fshort-enums -Wno-error=format \
	  -fno-exceptions -fno-enforce-eh-specs \
	  -nostdlib -ffreestanding -fno-threadsafe-statics \
	  -Wl,--gc-sections \
	  -Wno-error=unused-function -Os -fomit-frame-pointer -Wl,-z,nocopyreloc \
	  -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -u _printf_float


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

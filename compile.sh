#!/bin/bash

CC_PATH=~/workspace/bluenet/tools/gcc_arm_none_eabi/bin

SHARED_PATH=$HOME/workspace/bluenet/source/shared

export PATH=$PATH:$CC_PATH:$SHARED_PATH

export NRFX_LD=/home/anne/workspace/bluenet/tools/nrf5_sdk/modules/nrfx/mdk

FLAGS="-mthumb -ffunction-sections -fdata-sections -Wall -Werror -fno-strict-aliasing -fno-builtin -fshort-enums -Wno-error=format -Wno-error=unused-function -Os -fomit-frame-pointer -Wl,-z,nocopyreloc -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -u _printf_float"

echo arm-none-eabi-gcc ${FLAGS} main.c ${SHARED_PATH}/ipc/cs_IpcRamData.c -I${SHARED_PATH} -Tgeneric_gcc_nrf52.ld -o main.elf

arm-none-eabi-gcc ${FLAGS} main.c ${SHARED_PATH}/ipc/cs_IpcRamData.c -I${SHARED_PATH} -Tgeneric_gcc_nrf52.ld -o main.elf

arm-none-eabi-objcopy -O ihex main.elf main.hex

#arm-none-eabi-objdump -h main.elf

#address_main=$(arm-none-eabi-nm main.elf | grep -w main | cut -f1 -d' ')
#echo "Address of main is 0x$address_main"

address_main=$(arm-none-eabi-nm main.elf | grep -w dummy_main | cut -f1 -d' ')

echo "Address of dummy main is 0x$address_main"

#!/bin/bash

CC_PATH=~/workspace/bluenet/tools/gcc_arm_none_eabi/bin

SHARED_PATH=$HOME/workspace/bluenet/source/shared

export PATH=$PATH:$CC_PATH:$SHARED_PATH

export NRFX_LD=/home/anne/workspace/bluenet/tools/nrf5_sdk/modules/nrfx/mdk

address_main=$(arm-none-eabi-nm main.elf | grep -w dummy_main | cut -f1 -d' ')

echo "Address of dummy_main is 0x$address_main"
echo

echo "Show dummy_main function:"
arm-none-eabi-objdump -d main.elf | awk -F"\n" -v RS="\n\n" '$1 ~ /<dummy_main>/'

#echo
#echo "Now it does not start with eXX anymore. Thumb mode?"

#!/bin/bash

usage="Usage: $0 <input file>"

ifile=${1:? "$usage"}

size=$(stat --printf="%s" "$ifile")
size_plus_2=$(($size + 2))

# Use srec_cat to calculate
result=$(srec_cat $ifile --binary -crop 0 $size -fill 0xFF 0x0000 $size -crc16-b-e $size -poly 0x1021 -crop $size $size_plus_2 -o - -hex-dump)

crc=$(echo $result | cut -f2,3 -d' ' | tr -d ' ')
echo "0x$crc"

#!/bin/bash

usage="Usage: $0 <input file> <output file>"

ifile=${1:? "$usage"}
ofile=${2:? "$usage"}

checksum=$(./calc_nordic.sh "$ifile")
size=$(stat --printf="%s" "$ifile")

b_e_checksum=$(echo $checksum | sed 's/0x\(..\)\(..\)/\\x\2\\x\1/g')
b_e_size=$(printf "0x%04x" $size | sed 's/0x\(..\)\(..\)/\\x\2\\x\1/g')

protocol="\x00\x01"
header_size="\x14"
zeros="\x00\x00\x00\x00\x00\x00${header_size}\x00\x00\x00\x00\x00\x00\x00"

printf "${protocol}${b_e_size}${b_e_checksum}${zeros}" > "$ofile"

#hexdump "$ofile"

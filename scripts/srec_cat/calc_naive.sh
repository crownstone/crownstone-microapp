#!/bin/bash

usage="Usage: $0 <input file>"

ifile=${1:? "$usage"}

mkdir -p tmp
tmpfile1=tmp/temp1.bin

rm -f $tmpfile1

./remove_header.sh "$ifile" "$tmpfile1"

# Just calculate over the binary itself (no prefixing)
./crc_srec_cat.sh "$tmpfile1"

#cat ../../include/microapp_header_symbols.ld | grep -w CHECKSUM

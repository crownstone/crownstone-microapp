#!/bin/bash

ifile=../../build/example.bin

mkdir -p tmp
tmpfile1=tmp/temp1.bin

rm -f $tmpfile1

./remove_header.sh "$ifile" "$tmpfile1"

./crc_srec_cat.sh "$tmpfile1"

cat ../../include/microapp_header_symbols.ld | grep -w CHECKSUM

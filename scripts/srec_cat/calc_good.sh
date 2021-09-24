#!/bin/bash

ifile=../../build/example.bin

mkdir -p tmp
tmpfile1=tmp/temp1.bin
tmpfile2=tmp/temp2.bin

rm -f $tmpfile1
rm -f $tmpfile2

expected_crc=$(cat ../../include/microapp_header_symbols.ld | grep -w CHECKSUM | cut -f2 -d'=' | tr -d '; ')

echo "Expected CRC:"
printf "0x%x\n" $expected_crc

./remove_header.sh "$ifile" "$tmpfile1"

./prepend_bytes_good_to_bad_crc.sh "$tmpfile1" "$tmpfile2"

echo
echo "Calculated CRC with srec_cat:"
./crc_srec_cat.sh "$tmpfile2"


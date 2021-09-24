#!/bin/bash

usage="Usage: $0 <input file> <output file>"

ifile=${1:? "$usage"}
ofile=${2:? "$usage"}

mkdir -p tmp
tmpfile1=tmp/temp1.bin
tmpfile2=tmp/temp2.bin

rm -f $tmpfile1
rm -f $tmpfile2

# Generate header
./generate_header.sh "$ifile" "$tmpfile1"

# Prepend bytes to header
./prepend_bytes_good_to_bad_crc.sh "$tmpfile1" "$tmpfile2"

# Calculate CRC for header
header_crc=$(./crc_srec_cat.sh "$tmpfile2")

# Update header with checksum
./update_header.sh "$tmpfile1" "$header_crc"

# Remove header
./remove_header.sh "$ifile" "$tmpfile2"

# Generate final binary
cat "$tmpfile1" "$tmpfile2" > "$ofile"


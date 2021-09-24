#!/bin/bash

usage="Usage: $0 <input file> <output file>"

ifile=${1:? "$usage"}
ofile=${2:? "$usage"}

# Create trimmed binary by skipping the first 20 bytes
dd bs=20 skip=1 if="$ifile" of="$ofile" status=none
